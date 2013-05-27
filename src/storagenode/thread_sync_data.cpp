#include <new>
#include <list>
#include <queue>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "hash.h"
#include "file.h"
#include "rwlock.h"
#include "connector.h"
#include "msg_queue.h"
#include "vnode_info.h"
#include "tb_hashcode.h"
#include "timed_stream.h"
#include "storage_global.h"
#include "proxy_protocol.h"
#include "common_protocol.h"
#include "storage_protocol.h"
#include "thread_sync_data.h"
#include "file_reader_stream.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CRWLock g_sync_down_rwlock;
std::queue<int> g_sync_down_queue;

CThreadSyncData::CThreadSyncData(CMsgQueue *psync_msg_queue, CInetAddr master_addr) : \
	m_psync_msg_queue(psync_msg_queue), m_bstop_flag(false), m_master_addr(master_addr)
{

}

CThreadSyncData::~CThreadSyncData()
{

}

int CThreadSyncData::run()
{
	int ret, vnode_id;
	pkg_message *psync_pkg_msg;
	storage_sync_event *pstorage_sync_event;

#ifdef _DEBUG
	assert(m_psync_msg_queue != NULL);
#endif //_DEBUG

	m_bstop_flag = false;
	while(m_bstop_flag != true)
	{
		psync_pkg_msg = m_psync_msg_queue->get_msg();
#ifdef _DEBUG
		assert(psync_pkg_msg != NULL);
		assert(psync_pkg_msg->pkg_len == sizeof(storage_sync_event));
		assert(psync_pkg_msg->pkg_ptr != NULL);
#endif //_DEBUG
		pstorage_sync_event = (storage_sync_event *)(psync_pkg_msg->pkg_ptr);
		vnode_id = pstorage_sync_event->vnode_id;
		if((ret = do_sync_event(pstorage_sync_event)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"do vnode sync event failed, vnode_id: %d, errcode: %d", \
				__LINE__, vnode_id, ret);
			delete psync_pkg_msg;
			return ret;
		}
		g_sync_down_rwlock.wrlock();
		g_sync_down_queue.push(vnode_id);
		g_sync_down_rwlock.unlock();
		//KL_SYS_INFOLOG("vnode_id: %d sync down", vnode_id);
		delete psync_pkg_msg;
		psync_pkg_msg = NULL;
	}
	return ret;
}

int CThreadSyncData::do_sync_event(storage_sync_event *pstorage_sync_event)
{
	int vnode_id, ret;
	char *psync_dest_ip;
	int nsync_dest_port;
	CTimedStream *psync_stream;
	CConnector *psync_connector;

	vnode_id = pstorage_sync_event->vnode_id;
	psync_dest_ip = pstorage_sync_event->sync_dest_ip;
	nsync_dest_port = pstorage_sync_event->sync_dest_port;
	
	try
	{
		psync_connector = new CConnector(psync_dest_ip, nsync_dest_port);
	}
	catch(std::bad_alloc)
	{
		psync_connector = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create sync connector", \
			__LINE__);
		return ENOMEM;
	}
	catch(int errcode)
	{
		psync_connector = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create sync connector failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}
	try
	{
		psync_stream = new CTimedStream();
	}
	catch(std::bad_alloc)
	{
		psync_stream = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create sync stream", \
			__LINE__);
		delete psync_connector;
		return ENOMEM;
	}
	catch(int errcode)
	{
		psync_stream = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create sync stream failed, err: %s", \
			__LINE__, strerror(errcode));
		delete psync_connector;
		return errcode;
	}

	if(psync_connector->stream_connect(psync_stream) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"storage src server connecte to dest sync server failed, err: %s", \
			__LINE__, strerror(errno));
		delete psync_connector;
		delete psync_stream;
		return errno;
	}
	delete psync_connector;

	if((ret = check_and_sync_vnode(vnode_id, psync_stream)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"storage src server check and sync objects failed, dest ip: %s, " \
			"dest port: %d, errcode: %d", \
			__LINE__, psync_dest_ip, nsync_dest_port, ret);
		delete psync_stream;
		return ret;
	}
	delete psync_stream;
	return 0;
}

int CThreadSyncData::check_and_sync_vnode(int vnode_id, CTimedStream *psync_stream)
{
	int ret, i;
	int file_count;
	int64_t pkg_len;
	byte *psync_body;
	hashcode_list_t hc_list;
	pkg_header sync_header;
	CTbHashCode *ptb_hashcode;
	check_file_resp *pfile_check_resp;
	char db_path[KL_COMMON_PATH_LEN];
	sync_vnode_info *psync_vnode_info;
	check_file_info *pfile_check_info;

	snprintf(db_path, sizeof(db_path), \
		"%s/"KL_STORAGE_VNODE_NAME_PREFIX"%d/object/hashcode.db", \
		g_device_root, vnode_id);
	try
	{
		ptb_hashcode = new CTbHashCode(db_path);
	}
	catch(std::bad_alloc)
	{
		ptb_hashcode = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create tb_hashcode obj", \
			__LINE__);
		return ENOMEM;
	}
	catch(int errcode)
	{
		ptb_hashcode = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create tb_hashcode obj failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}
	catch(const char *perrmsg)
	{
		ptb_hashcode = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create tb_hashcode obj failed, err: %s", \
			__LINE__, perrmsg);
		return -1;
	}
	//get all data file hash code
	if((ret = ptb_hashcode->query_all_hashcode(&hc_list)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"query all hash code record failed, errcode: %d", \
			__LINE__, ret);
		delete ptb_hashcode;
		return ret;
	}
	delete ptb_hashcode;

	pkg_len = sizeof(sync_vnode_info) + sizeof(check_file_info) * (hc_list.size() + 3);
	try
	{
		psync_body = new byte[pkg_len];
	}
	catch(std::bad_alloc)
	{
		psync_body = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d", \
			"no more memory to create sync body", \
			__LINE__);
		return ENOMEM;
	}

	//KL_SYS_INFOLOG("file: "__FILE__", line: %d, sync vnode_id: %d", __LINE__, vnode_id);
	psync_vnode_info = (sync_vnode_info *)psync_body;
	CSERIALIZER::int2buff(vnode_id, psync_vnode_info->vnode_id);
	g_pcontainer_rwlock->rdlock();
	CSERIALIZER::long2buff(g_pstorage_vnode_container->get_vnode(vnode_id)->vnode_version, \
		psync_vnode_info->vnode_version);
	g_pcontainer_rwlock->unlock();
	pfile_check_info = (check_file_info *)(psync_body + sizeof(sync_vnode_info));
	while(!hc_list.empty())
	{
		memcpy(pfile_check_info->file_path, hc_list.front().file_path, KL_COMMON_PATH_LEN);
		memcpy(pfile_check_info->hash_code, hc_list.front().hash_code, KL_COMMON_MD5_HASH_LEN);
		hc_list.pop_front();
		pfile_check_info++;
	}
	try
	{
		//get delete file record db hashcode
		memset(db_path, 0, KL_COMMON_PATH_LEN);
		snprintf(db_path, KL_COMMON_PATH_LEN, KL_STORAGE_VNODE_NAME_PREFIX \
			"%d/object/delete_file_record.db", vnode_id);
		strcpy((char*)(pfile_check_info->file_path), db_path);
		memset(db_path, 0, KL_COMMON_PATH_LEN);
		snprintf(db_path, KL_COMMON_PATH_LEN, "%s/"KL_STORAGE_VNODE_NAME_PREFIX \
			"%d/object/delete_file_record.db", g_device_root, vnode_id);

		MD5 md5;
		CFile delete_record_db(db_path, O_RDONLY);
		if((ret = md5.update(&delete_record_db)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"get delete record db file hashcode failed, err: %s", \
				__LINE__, strerror(ret));
			delete psync_body;
			return ret;
		}
		md5.to_string((char*)(pfile_check_info->hash_code), KL_COMMON_MD5_HASH_LEN);
		pfile_check_info++;
		//get account db hashcode
		memset(db_path, 0, KL_COMMON_PATH_LEN);
		snprintf(db_path, KL_COMMON_PATH_LEN, KL_STORAGE_VNODE_NAME_PREFIX \
			"%d/account/account.db", vnode_id);
		strcpy((char*)(pfile_check_info->file_path), db_path);
		memset(db_path, 0, KL_COMMON_PATH_LEN);
		snprintf(db_path, KL_COMMON_PATH_LEN, "%s/"KL_STORAGE_VNODE_NAME_PREFIX \
			"%d/account/account.db", g_device_root, vnode_id);
		CFile account_db(db_path, O_RDONLY);
		md5.reset();
		if((ret = md5.update(&account_db)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"get account db file hashcode failed, err: %s", \
				__LINE__, strerror(ret));
			delete psync_body;
			return ret;
		}
		md5.to_string((char*)(pfile_check_info->hash_code), KL_COMMON_MD5_HASH_LEN);
		pfile_check_info++;
		//get container db hashcode
		memset(db_path, 0, KL_COMMON_PATH_LEN);
		snprintf(db_path, KL_COMMON_PATH_LEN, KL_STORAGE_VNODE_NAME_PREFIX \
			"%d/container/container.db", vnode_id);
		strcpy((char*)(pfile_check_info->file_path), db_path);
		memset(db_path, 0, KL_COMMON_PATH_LEN);
		snprintf(db_path, KL_COMMON_PATH_LEN, "%s/"KL_STORAGE_VNODE_NAME_PREFIX \
			"%d/container/container.db", g_device_root, vnode_id);
		CFile container_db(db_path, O_RDONLY);
		md5.reset();
		if((ret = md5.update(&container_db)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"get container db file hashcode failed, err: %s", \
				__LINE__, strerror(ret));
			delete psync_body;
			return ret;
		}
		md5.to_string((char*)(pfile_check_info->hash_code), KL_COMMON_MD5_HASH_LEN);
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create db file", \
			__LINE__);
		delete psync_body;
		return ENOMEM;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create db file failed, err: %s", \
			__LINE__, strerror(errcode));
		delete psync_body;
		return errcode;
	}

	sync_header.cmd = KL_STORAGE_CMD_FILE_CHECK;
	sync_header.status = 0;
	CSERIALIZER::long2buff(pkg_len, sync_header.pkg_len);
	if((ret = psync_stream->stream_send(&sync_header, sizeof(sync_header), \
		g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"sync stream send sync header failed, err: %s", \
			__LINE__, strerror(ret));
		delete psync_body;
		return ret;
	}
	if((ret = psync_stream->stream_send(psync_body, pkg_len, g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"sync stream send sync body failed, err: %s", \
			__LINE__, strerror(ret));
		delete psync_body;
		return ret;
	}
	delete psync_body;

	if((ret = psync_stream->stream_recv(&sync_header, sizeof(sync_header), \
		g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"sync stream recv response header failed, err: %s", \
			__LINE__, strerror(ret));
		return ret;
	}
	if(sync_header.status != 0)
	{
		return sync_header.status;
	}
	pkg_len = CSERIALIZER::buff2int64(sync_header.pkg_len);
	if(pkg_len == 0)
	{
		return 0;
	}
	if(pkg_len % sizeof(check_file_resp) != 0)
	{
		return EINVAL;
	}

	try
	{
		psync_body = new byte[pkg_len];
	}
	catch(std::bad_alloc)
	{
		psync_body = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create response body", \
			__LINE__);
		return ENOMEM;
	}
	if((ret = psync_stream->stream_recv(psync_body, pkg_len, g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"sync stream recv response body failed, err: %s", \
			__LINE__, strerror(ret));
		delete psync_body;
		return ret;
	}

	//start to sync file
	file_count = pkg_len / sizeof(check_file_resp);
	pfile_check_resp = (check_file_resp *)psync_body;
	for(i = 0; i < file_count; i++)
	{
		if((ret = sync_vnode_file((char*)((pfile_check_resp + i)->file_path), psync_stream)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"storage node sync file failed, errcode: %d", \
				__LINE__, ret);
			delete psync_body;
			return ret;
		}
	}
	delete psync_body;
	return 0;
}

int CThreadSyncData::sync_vnode_file(const char *path, CTimedStream *psync_stream)
{
	int ret;
	int64_t pkg_len;
	pkg_header sync_file_header;
	sync_file_info file_info;
	CFileReaderStream *pfile_reader_stream;
	char true_file_path[KL_COMMON_PATH_LEN];

	memset(true_file_path, 0, KL_COMMON_PATH_LEN);
	snprintf(true_file_path, KL_COMMON_PATH_LEN, "%s/%s", g_device_root, path);
	try
	{
		pfile_reader_stream = new CFileReaderStream(true_file_path, psync_stream);
	}
	catch(std::bad_alloc)
	{
		pfile_reader_stream = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create file reader stream", \
			__LINE__);
		return ENOMEM;
	}
	catch(int errcode)
	{
		pfile_reader_stream = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create file reader stream failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}

	pkg_len = sizeof(sync_file_info);
	CSERIALIZER::long2buff(pkg_len, sync_file_header.pkg_len);
	sync_file_header.cmd = KL_STORAGE_CMD_FILE_SYNC;
	sync_file_header.status = 0;
	if((ret = psync_stream->stream_send(&sync_file_header, sizeof(sync_file_header), \
		g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"sync stream send sync file header failed, err: %s", \
			__LINE__, strerror(ret));
		delete pfile_reader_stream;
		return ret;
	}
	strcpy((char*)(file_info.file_path), path);
	CSERIALIZER::long2buff(pfile_reader_stream->get_file_size(), file_info.file_length);
	if((ret = psync_stream->stream_send(&file_info, sizeof(file_info), g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"sync stream send sync file info failed, err: %s", \
			__LINE__, strerror(ret));
		delete pfile_reader_stream;
		return ret;
	}
	if((ret = pfile_reader_stream->stream_readout()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"src storage node send file data failed, err: %s", \
			__LINE__, strerror(errno));
		delete pfile_reader_stream;
		return errno;
	}
	if((ret = psync_stream->stream_recv(&sync_file_header, sizeof(pkg_header), g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"src storage node recv response header failed, err: %s", \
			__LINE__, strerror(ret));
		delete pfile_reader_stream;
		return ret;
	}
	delete pfile_reader_stream;
	return sync_file_header.status;
}

int CThreadSyncData::stop()
{
	m_bstop_flag = true;
}