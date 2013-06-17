#include <new>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "log.h"
#include "hash.h"
#include "file.h"
#include "rwlock.h"
#include "directory.h"
#include "connector.h"
#include "vnode_info.h"
#include "tb_hashcode.h"
#include "timed_stream.h"
#include "storage_global.h"
#include "proxy_protocol.h"
#include "storage_protocol.h"
#include "storage_msg_parser.h"
#include "file_reader_stream.h"
#include "file_writer_stream.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CStorageMsgParser::CStorageMsgParser()
{
}

CStorageMsgParser::~CStorageMsgParser()
{
}

int CStorageMsgParser::parse_msg(pkg_message* pkg_msg_ptr)
{
#ifdef _DEBUG
	assert(pkg_msg_ptr);
#endif //_DEBUG
	byte msg_cmd;
	if(pkg_msg_ptr->pkg_ptr == NULL)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"the context of message package is null", \
			__LINE__);
		return -1;
	}

	msg_cmd = *(pkg_msg_ptr->pkg_ptr);
	switch (msg_cmd)
	{
	case KL_STORAGE_CMD_FILE_CHECK:
		return msg_file_check_handle(pkg_msg_ptr);
	case KL_STORAGE_CMD_FILE_SYNC:
		return msg_file_sync_handle(pkg_msg_ptr);
	case KL_STORAGE_CMD_UPLOAD_FILE:
		return msg_upload_file_handle(pkg_msg_ptr);
	case KL_STORAGE_CMD_CTRL_STREAM:
		return msg_ctrl_stream_handle(pkg_msg_ptr);
	default:
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"storage msg parser catch an undefined msg cmd(cmd = %d)", \
			__LINE__, msg_cmd);
		delete pkg_msg_ptr;
		return EINVAL;
	}
	return -1;
}

int CStorageMsgParser::msg_file_check_handle(pkg_message* pkg_msg_ptr)
{
	int ret, i, res;
	byte *presp_body;
	int64_t resp_pkg_len;
	int nsync_file_count;
	int64_t vnode_version;
	pkg_header resp_header;
	int vnode_id, file_count;
	CTimedStream *presp_stream;
	storage_vnode *pstorage_vnode;
	psync_vnode_info vnode_info_ptr;
	pcheck_file_info check_file_ptr;
	hashcode_list_t file_hashcode_list;

	if((pkg_msg_ptr->pkg_len - sizeof(sync_vnode_info) - 2) % \
		sizeof(check_file_info) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the msg pkg of file check handle is illegal", \
			__LINE__);
		delete pkg_msg_ptr;
		return ret = EINVAL;
	}
	file_count = (pkg_msg_ptr->pkg_len - sizeof(sync_vnode_info) - 2) / sizeof(check_file_info);
	try
	{
		presp_stream = new CTimedStream(pkg_msg_ptr->sock_stream_fd, false);
	}
	catch(std::bad_alloc)
	{
		presp_stream = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create response stream", \
			__LINE__);
		delete pkg_msg_ptr;
		return ENOMEM;
	}
	catch(int errcode)
	{
		presp_stream = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create response stream failed, err: %s", \
			__LINE__, strerror(errcode));
		delete pkg_msg_ptr;
		return errcode;
	}

	vnode_info_ptr = (psync_vnode_info)(pkg_msg_ptr->pkg_ptr + 2);
	vnode_id = CSERIALIZER::buff2int32(vnode_info_ptr->vnode_id);
	vnode_version = CSERIALIZER::buff2int64(vnode_info_ptr->vnode_version);
	ret = g_pcontainer_rwlock->rdlock();
#ifdef _DEBUG
	assert(ret == 0);
#endif //_DEBUG
	pstorage_vnode = g_pstorage_vnode_container->get_vnode(vnode_id);
	ret = g_pcontainer_rwlock->unlock();
#ifdef _DEBUG
	assert(ret == 0);
#endif //_DEBUG
	if(pstorage_vnode != NULL)
	{
		pstorage_vnode->vnode_status = KL_REPLICA_STATUS_WAIT_SYNC;
		if(pstorage_vnode->vnode_status == vnode_version)
		{
			nsync_file_count = 0;
			goto do_success_resp;
		}
		pstorage_vnode->vnode_version = vnode_version;
	}
	else
	{
		//add storage vnode to container
		try
		{
			pstorage_vnode = new storage_vnode();
		}
		catch(std::bad_alloc)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"no more memory to create storage vnode", \
				__LINE__);
			ret = ENOMEM;
			goto do_err_resp;
		}
		pstorage_vnode->vnode_id = vnode_id;
		pstorage_vnode->vnode_status = KL_REPLICA_STATUS_WAIT_SYNC;
		pstorage_vnode->vnode_version = vnode_version;
		ret = g_pcontainer_rwlock->wrlock();
#ifdef _DEBUG
		assert(ret == 0);
#endif //_DEBUG
		if((ret = g_pstorage_vnode_container->add_vnode(pstorage_vnode)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"file check handle add vnode to vnode container failed, " \
				"vnode_id: %d, err: %s", \
				__LINE__, vnode_id, strerror(ret));
			g_pcontainer_rwlock->unlock();
			goto do_err_resp;
		}
		ret = g_pcontainer_rwlock->unlock();
#ifdef _DEBUG
		assert(ret == 0);
#endif //_DEBUG
	}

	nsync_file_count = 0;
	presp_body = new byte[file_count * sizeof(check_file_resp)];
	check_file_ptr = (pcheck_file_info)(pkg_msg_ptr->pkg_ptr + \
		sizeof(sync_vnode_info) + 2);
	try
	{
		int j;
		bool bsync_flag;
		pcheck_file_resp pfile_resp_info;
		char db_path[KL_COMMON_PATH_LEN];
		char file_path[KL_COMMON_PATH_LEN];
		snprintf(db_path, KL_COMMON_PATH_LEN, "%s/"KL_STORAGE_VNODE_NAME_PREFIX \
			"%d/object/hashcode.db", g_device_root, vnode_id);
		CTbHashCode tb_hashcode(db_path);
		tb_hashcode.query_all_hashcode(&file_hashcode_list);

		for(i = 0; i < file_count; i++)
		{
			//check whether local file hashcode exist
			hashcode_iter_t hashcode_iter;
			for(hashcode_iter = file_hashcode_list.begin(); \
				hashcode_iter != file_hashcode_list.end(); hashcode_iter++)
			{
				if(strcmp((*hashcode_iter).file_path, \
					(const char*)(check_file_ptr->file_path)) == 0)
				{
					file_hashcode_list.erase(hashcode_iter);
					break;
				}
			}
			if((ret = do_file_hashcode_check(check_file_ptr, bsync_flag)) != 0)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"do file hashcode check failed, err: %s", \
					__LINE__, strerror(ret));
				delete [] presp_body;
				goto do_err_resp;
			}
			if(bsync_flag == true)
			{
				pfile_resp_info = (pcheck_file_resp)presp_body;
				strcpy((char*)(pfile_resp_info->file_path), \
					(const char*)(check_file_ptr->file_path));
				nsync_file_count++;
				pfile_resp_info++;
			}
			check_file_ptr++;
		}
		hashcode_iter_t hashcode_iter;
		for(hashcode_iter = file_hashcode_list.begin(); \
			hashcode_iter != file_hashcode_list.end(); hashcode_iter++)
		{
			memset(file_path, 0, KL_COMMON_PATH_LEN);
			snprintf(file_path, KL_COMMON_PATH_LEN, "%s/%s", \
				g_device_root, \
				(*hashcode_iter).file_path);

			tb_hashcode.delete_record((*hashcode_iter).file_path);
			CFile().unlink_file(file_path); //delete file
		}
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to query all local file hashcode", \
			__LINE__);
		ret = ENOMEM;
		delete [] presp_body;
		goto do_err_resp;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"query all local file hashcode failed, err: %s", \
			__LINE__, strerror(errcode));
		ret = errcode;
		delete [] presp_body;
		goto do_err_resp;
	}
	catch(const char *perrmsg)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"query all local file hashcode failed, err: %s", \
			__LINE__, perrmsg);
		ret = ENOMEM;
		delete [] presp_body;
		goto do_err_resp;
	}
	goto do_success_resp;
	
do_err_resp:
	resp_header.cmd = KL_STORAGE_CMD_SERVER_RESP;
	resp_header.status = ret;
	CSERIALIZER::long2buff(0, resp_header.pkg_len);
	if((res = presp_stream->stream_send(&resp_header, sizeof(pkg_header), g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"response stream send resp header failed, err: %s", \
			__LINE__, strerror(res));
	}
	delete pkg_msg_ptr;
	delete presp_stream;
	return ret;
do_success_resp:
	resp_header.cmd = KL_STORAGE_CMD_SERVER_RESP;
	resp_header.status = 0;
	resp_pkg_len = nsync_file_count * sizeof(check_file_resp);
	CSERIALIZER::long2buff(resp_pkg_len, resp_header.pkg_len);
	if((ret = presp_stream->stream_send(&resp_header, sizeof(pkg_header), g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"response stream send resp header failed, err: %s", \
			__LINE__, strerror(ret));
		delete pkg_msg_ptr;
		delete presp_stream;
		delete [] presp_body;
		return ret;
	}
	if(resp_pkg_len != 0)
	{
		if((ret = presp_stream->stream_send(presp_body, resp_pkg_len, g_ntimeout)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"response stream send resp body failed, err: %s", \
				__LINE__, strerror(ret));
		}
	}
	delete pkg_msg_ptr;
	delete presp_stream;
	delete [] presp_body;
	return ret;
}

int CStorageMsgParser::msg_file_sync_handle(pkg_message* pkg_msg_ptr)
{
	int ret, i;
	int nfile_count;
	int64_t nfile_size;
	pkg_header resp_header;
	CTimedStream *presp_stream;
	psync_file_info file_info_ptr;
	char true_file_path[KL_COMMON_PATH_LEN];

	if((pkg_msg_ptr->pkg_len - 2) % sizeof(sync_file_info) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the msg pkg of file sync handle is illegal", \
			__LINE__);
		delete pkg_msg_ptr;
		return ret = EINVAL;
	}

	try
	{
		presp_stream = new CTimedStream(pkg_msg_ptr->sock_stream_fd, false);
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create resp stream", \
			__LINE__);
		ret = ENOMEM;
		goto do_resp;
	}
	
	ret = 0;
	nfile_count = (pkg_msg_ptr->pkg_len - 2) / sizeof(sync_file_info);
	file_info_ptr = (psync_file_info)(pkg_msg_ptr->pkg_ptr + 2);
	for(i = 0; i < nfile_count; i++)
	{
		memset(true_file_path, 0, KL_COMMON_PATH_LEN);
		snprintf(true_file_path, KL_COMMON_PATH_LEN, "%s/%s", \
			g_device_root, (const char *)(file_info_ptr->file_path));
		try
		{
			CFileWriterStream file_writer_stream(true_file_path, presp_stream);
			nfile_size = CSERIALIZER::buff2int64(file_info_ptr->file_length);
			ret = file_writer_stream.stream_writein(nfile_size);
			if(ret != 0)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"write data to file failed, file_path: %s, err: %s", \
					__LINE__, true_file_path, strerror(ret));
				goto do_resp;
			}
		}
		catch(std::bad_alloc)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"no more memory to write data to file, file_path: %s", \
				__LINE__, true_file_path);
			ret = ENOMEM;
			goto do_resp;
		}
		catch(int errcode)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"write data to file failed, file_path: %s, err: %s", \
				__LINE__, true_file_path, strerror(errcode));
			ret = errcode;
			goto do_resp;
		}
		file_info_ptr++;
	}
do_resp:
	resp_header.cmd = KL_STORAGE_CMD_SERVER_RESP;
	resp_header.status = ret;
	CSERIALIZER::long2buff(0, resp_header.pkg_len);
	if((ret = presp_stream->stream_send(&resp_header, sizeof(pkg_header), g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"resp stream send response header failed, err: %s", \
			__LINE__, strerror(ret));
	}
	delete pkg_msg_ptr;
	delete presp_stream;
	return ret;
}

int CStorageMsgParser::msg_upload_file_handle(pkg_message *pkg_msg_ptr)
{
	int ret, res;
	int nvnode_id, i;
	int nstorage_port;
	int64_t nfile_length;
	int nctrl_stream_count;
	pkg_header resp_header;
	int nsecond_replica_count;
	CTimedStream *presp_stream;
	storage_vnode *pstorage_vnode;
	pstorage_info storage_info_ptr;
	pupload_file_info upload_file_ptr;
	char true_file_path[KL_COMMON_PATH_LEN];

	if((pkg_msg_ptr->pkg_len - 2 - sizeof(upload_file_info)) % sizeof(storage_info) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the msg pkg of upload file is illegal, pkg len: %d", \
			__LINE__, pkg_msg_ptr->pkg_len);
		delete pkg_msg_ptr;
		return EINVAL;
	}

	try
	{
		presp_stream = new CTimedStream(pkg_msg_ptr->sock_stream_fd, false);
	}
	catch(std::bad_alloc)
	{
		presp_stream = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create upload file resp stream", \
			__LINE__);
		delete pkg_msg_ptr;
		return ENOMEM;
	}
	catch(int errcode)
	{
		presp_stream = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create upload file resp stream failed, err: %s", \
			__LINE__, strerror(errcode));
		delete pkg_msg_ptr;
		return errcode;
	}
	
	upload_file_ptr = (pupload_file_info)(pkg_msg_ptr->pkg_ptr + 2);
	nvnode_id = CSERIALIZER::buff2int32(upload_file_ptr->vnode_id);
	nfile_length = CSERIALIZER::buff2int64(upload_file_ptr->file_length);
	memset(true_file_path, 0, KL_COMMON_PATH_LEN);
	if((ret = g_pcontainer_rwlock->rdlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"rdlock storage vnode container rwlock failed, err: %s", \
			__LINE__, strerror(ret));
		delete pkg_msg_ptr;
		delete presp_stream;
		return ret;
	}
	if((pstorage_vnode = g_pstorage_vnode_container->get_vnode(nvnode_id)) == NULL)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"not found vnode info(vnode_id: %d) in storage vnode container", \
			__LINE__, nvnode_id);
		g_pcontainer_rwlock->unlock();
		ret = EINVAL;
		goto do_err_resp;
	}
	if((ret = g_pcontainer_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"unlock container rwlock failed, err: %s", \
			__LINE__, strerror(ret));
		goto do_err_resp;
	}
	if(pstorage_vnode->vnode_status != KL_REPLICA_STATUS_ACTIVE)
	{
		ret = EACCES;
		goto do_err_resp;
	}
	snprintf(true_file_path, KL_COMMON_PATH_LEN, "%s/"KL_STORAGE_VNODE_NAME_PREFIX"%d/" \
		"object/%s", \
		g_device_root, nvnode_id, (char *)(upload_file_ptr->file_path));
	try
	{
		const char *p;
		char dir_path[KL_COMMON_PATH_LEN];

		memset(dir_path, 0, KL_COMMON_PATH_LEN);
		p = strrchr(true_file_path, '/');
		memcpy(dir_path, true_file_path, p - true_file_path);
		if((ret = CDirectory().make_dir(dir_path)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"storage writer stream create file dir(dir_path: %s) failed, err: %s", \
				__LINE__, dir_path, strerror(ret));
			goto do_err_resp;
		}
		CFileWriterStream file_writer_stream(true_file_path, presp_stream);
		
		if((ret = file_writer_stream.stream_writein(nfile_length)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"file writer stream recv file(file_path: %s) failed, err: %s", \
				__LINE__, true_file_path, strerror(ret));
			goto do_err_resp;
		}
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more to write file to file_path: %s", \
			__LINE__, true_file_path);
		ret = ENOMEM;
		goto do_err_resp;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"write file to file_path: %s failed, err: %s", \
			__LINE__, true_file_path, strerror(errcode));
		ret = errcode;
		goto do_err_resp;
	}
	
	if((ret = g_pcontainer_rwlock->wrlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"wrlock storage node container rwlock failed, err: %s", \
			__LINE__, strerror(ret));
		goto do_err_resp;
	}
	pstorage_vnode->vnode_version ++;
	//write to other storage node
	nsecond_replica_count = (pkg_msg_ptr->pkg_len - 2 - sizeof(upload_file_info)) \
		/ sizeof(storage_info);
	if(nsecond_replica_count < 1)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"write file need two storage replica at least, vnode_id: %d, file_path: %s", \
			__LINE__, nvnode_id, true_file_path);
		ret = EINVAL;
		g_pcontainer_rwlock->unlock();
		goto do_err_resp;
	}
	nctrl_stream_count = 0;
	storage_info_ptr = (pstorage_info)(pkg_msg_ptr->pkg_ptr + sizeof(upload_file_info) + 2);
	for(i = 0; i < nsecond_replica_count; i++)
	{
		nstorage_port = CSERIALIZER::buff2int32(storage_info_ptr->device_bind_port);
		try
		{
			pkg_header write_header;
			CTimedStream file_write_stream;
			CConnector storage_connector((const char *)(storage_info_ptr->device_bind_ip), \
				nstorage_port);
			if((ret = storage_connector.stream_connect(&file_write_stream)) != 0)
			{
				KL_SYS_NOTICELOG("file: "__FILE__", line: %d, " \
					"primary storage node connect to second storage node(ip: %s, port: %d) " \
					"failed, err: %s", \
					__LINE__, (const char *)(storage_info_ptr->device_bind_ip), \
					nstorage_port, strerror(ret));
				storage_info_ptr++;
				continue;
			}
			CSERIALIZER::long2buff(sizeof(upload_file_info), write_header.pkg_len);
			write_header.cmd = KL_STORAGE_CMD_CTRL_STREAM;
			write_header.status = 0;
			if((ret = file_write_stream.stream_send(&write_header, sizeof(pkg_header), \
				g_ntimeout)) != 0)
			{
				KL_SYS_NOTICELOG("file: "__FILE__", line: %d, " \
					"primary storage node send msg header to second storage " \
					"node(ip: %s, port: %d) failed, err: %s", \
					__LINE__, (const char *)(storage_info_ptr->device_bind_ip), \
					nstorage_port, strerror(ret));
				storage_info_ptr++;
				continue;
			}
			CFileReaderStream file_reader_stream(true_file_path, &file_write_stream);
			if((ret = file_reader_stream.stream_readout()) != 0)
			{
				KL_SYS_NOTICELOG("file: "__FILE__", line: %d, " \
					"primary storage node write file to seconde storage node(ip: %s,  " \
					"port: %d) failed, err: %s", \
					__LINE__, (const char *)(storage_info_ptr->device_bind_ip), \
					nstorage_port, strerror(ret));
				storage_info_ptr++;
				continue;
			}
			if((ret = file_write_stream.stream_recv(&write_header, sizeof(pkg_header), \
				g_ntimeout)) != 0)
			{
				KL_SYS_NOTICELOG("file: "__FILE__", line: %d, " \
					"primary storage node recv msg header from second storage " \
					"node(ip: %s, port: %d) failed, err: %s", \
					__LINE__, (const char *)(storage_info_ptr->device_bind_ip), \
					nstorage_port, strerror(ret));
				storage_info_ptr++;
				continue;
			}
			if(write_header.status != 0)
			{
				KL_SYS_NOTICELOG("file: "__FILE__", line: %d, " \
					"primary storage node write file to second storage " \
					"node(ip: %s, port: %d) failed, resp status: %d", \
					__LINE__, (const char *)(storage_info_ptr->device_bind_ip), \
					nstorage_port, write_header.status);
				storage_info_ptr++;
				continue;
			}
		}
		catch(std::bad_alloc)
		{
			KL_SYS_NOTICELOG("file: "__FILE__", line: %d, " \
				"no more memory to support primary storage node write file to second storage " \
				"node(ip, port: %d)", \
				__LINE__, (const char *)(storage_info_ptr->device_bind_ip), nstorage_port);
			storage_info_ptr++;
			continue;
		}
		catch(int errcode)
		{
			KL_SYS_NOTICELOG("file: "__FILE__", line: %d, " \
				"primary storage node write file to second storage node(ip: %s, port: %d) " \
				"failed, err: %s", \
				__LINE__, (const char *)(storage_info_ptr->device_bind_ip), nstorage_port, \
				strerror(errcode));
			storage_info_ptr++;
			continue;
		}
		nctrl_stream_count++;
		storage_info_ptr++;
	}
	if((ret = g_pcontainer_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"unlock container rwlock failed, err: %s", \
			__LINE__, strerror(ret));
		goto do_err_resp;
	}
	if(nctrl_stream_count < 1)
	{
		ret = EACCES;
		goto do_err_resp;
	}
	//write success
	ret = 0;
	goto do_succ_resp;

do_err_resp:
	CSERIALIZER::long2buff(0, resp_header.pkg_len);
	resp_header.cmd = KL_STORAGE_CMD_SERVER_RESP;
	resp_header.status = ret;
	if((res = presp_stream->stream_send(&resp_header, sizeof(pkg_header), \
		g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"resp stream send upload file response header failed, err: %s", \
			__LINE__, strerror(res));
	}
	delete pkg_msg_ptr;
	delete presp_stream;
	return ret;
do_succ_resp:
	CSERIALIZER::long2buff(0, resp_header.pkg_len);
	resp_header.cmd = KL_STORAGE_CMD_SERVER_RESP;
	resp_header.status = 0;
	if((ret = presp_stream->stream_send(&resp_header, sizeof(pkg_header), \
		g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"resp stream send upload file response header failed, err: %s", \
			__LINE__, strerror(ret));
	}
	delete pkg_msg_ptr;
	delete presp_stream;
	return ret;
}

int CStorageMsgParser::msg_download_file_handle(pkg_message *pkg_msg_ptr)
{
	int ret, res;
	int nvnode_id;
	int64_t nfile_length;
	pkg_header resp_header;
	CTimedStream *presp_stream;
	storage_vnode *storage_vnode_ptr;
	pdownload_file_info download_info_ptr;
	char true_file_path[KL_COMMON_PATH_LEN];

	if(pkg_msg_ptr->pkg_len - 2 != sizeof(download_file_info))
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the msg pkg of download file is illegal, pkg_len: %d", \
			__LINE__, pkg_msg_ptr->pkg_len - 2);
		delete pkg_msg_ptr;
		return EINVAL;
	}

	download_info_ptr = (pdownload_file_info)(pkg_msg_ptr->pkg_ptr + 2);
	nvnode_id = CSERIALIZER::buff2int32(download_info_ptr->vnode_id);
	memset(true_file_path, 0, KL_COMMON_PATH_LEN);
	try
	{
		presp_stream = new CTimedStream(pkg_msg_ptr->sock_stream_fd, false);
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create download file response stream", \
			__LINE__);
		delete pkg_msg_ptr;
		return ENOMEM;
	}

	if((ret = g_pcontainer_rwlock->rdlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"rdlock storage node container rwlock failed, err: %s", \
			__LINE__, strerror(ret));
		goto do_err_resp;
	}
	if((storage_vnode_ptr = g_pstorage_vnode_container->get_vnode(nvnode_id)) == NULL)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"not found vnode info(vnode_id: %d) in storage vnode container", \
			__LINE__, nvnode_id);
		g_pcontainer_rwlock->unlock();
		ret = EINVAL;
		goto do_err_resp;
	}
	if((ret = g_pcontainer_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"unlock storage node container rwlock failed, err: %s", \
			__LINE__, strerror(ret));
		goto do_err_resp;
	}
	if(storage_vnode_ptr->vnode_status != KL_REPLICA_STATUS_ACTIVE)
	{
		ret = EACCES;
		goto do_err_resp;
	}
	//try to read file
	snprintf(true_file_path, KL_COMMON_PATH_LEN, "%s/"KL_STORAGE_VNODE_NAME_PREFIX"%d/" \
		"object/%s", \
		g_device_root, nvnode_id, (const char *)(download_info_ptr->file_path));
	try
	{
		download_file_resp download_resp;
		CFileReaderStream file_reader_stream(true_file_path, presp_stream);

		nfile_length = file_reader_stream.get_file_size();
		CSERIALIZER::long2buff(sizeof(download_file_resp), resp_header.pkg_len);
		resp_header.cmd = KL_STORAGE_CMD_SERVER_RESP;
		resp_header.status = 0;
		if((ret = presp_stream->stream_send(&resp_header, sizeof(pkg_header), g_ntimeout)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"storage node send response header failed, err: %s", \
				__LINE__, strerror(ret));
			delete pkg_msg_ptr;
			delete presp_stream;
			return ret;
		}

		CSERIALIZER::long2buff(nfile_length, download_resp.file_length);
		if((ret = presp_stream->stream_send(&download_resp, sizeof(download_file_resp), \
			g_ntimeout)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"storage node send response body failed, err: %s", \
				__LINE__, strerror(ret));
			delete pkg_msg_ptr;
			delete presp_stream;
			return ret;
		}
		if((ret = file_reader_stream.stream_readout()) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"storage node send file data filed, err: %s", \
				__LINE__, strerror(ret));
		}
		delete pkg_msg_ptr;
		delete presp_stream;
		return ret;
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to service to download file, file_path: %s", \
			__LINE__, true_file_path);
		ret = ENOMEM;
		goto do_err_resp;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"client download file(file_path: %s) failed, err: %s", \
			__LINE__, true_file_path, strerror(errcode));
		ret = errcode;
		goto do_err_resp;
	}

do_err_resp:
	CSERIALIZER::long2buff(0, resp_header.pkg_len);
	resp_header.cmd = KL_STORAGE_CMD_SERVER_RESP;
	resp_header.status = ret;
	if((res = presp_stream->stream_send(&resp_header, sizeof(pkg_header), g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"storage download file response stream send resp header failed, err: %s", \
			__LINE__, strerror(res));
	}
	delete pkg_msg_ptr;
	delete presp_stream;
	return ret;
}

int CStorageMsgParser::msg_ctrl_stream_handle(pkg_message *pkg_msg_ptr)
{
	int ret, res;
	int nvnode_id;
	int64_t nfile_length;
	pkg_header resp_header;
	CTimedStream *presp_stream;
	storage_vnode *storage_vnode_ptr;
	pupload_file_info upload_file_ptr;
	char true_file_path[KL_COMMON_PATH_LEN];

	if(pkg_msg_ptr->pkg_len - 2 != sizeof(upload_file_info))
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"ctrl stream handle catch an illegal msg pkg", \
			__LINE__);
		delete pkg_msg_ptr;
		return EINVAL;
	}

	try
	{
		presp_stream = new CTimedStream(pkg_msg_ptr->sock_stream_fd, false);
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create response stream", \
			__LINE__);
		delete pkg_msg_ptr;
		return ENOMEM;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create ctrl response stream failed, err: %s", \
			__LINE__, strerror(errcode));
		delete pkg_msg_ptr;
		return errcode;
	}

	upload_file_ptr = (pupload_file_info)(pkg_msg_ptr->pkg_ptr + 2);
	nvnode_id = CSERIALIZER::buff2int32(upload_file_ptr->vnode_id);
	nfile_length = CSERIALIZER::buff2int64(upload_file_ptr->file_length);
	memset(true_file_path, 0, KL_COMMON_PATH_LEN);
	if((ret = g_pcontainer_rwlock->rdlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"rdlock storage vnode container rwlock failed, err: %s", \
			__LINE__, strerror(ret));
		goto do_err_resp;
	}
	if((storage_vnode_ptr = g_pstorage_vnode_container->get_vnode(nvnode_id)) == NULL)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"not found vnode info(vnode_id: %d) in storage vnode container", \
			__LINE__, nvnode_id);
		g_pcontainer_rwlock->unlock();
		ret = EINVAL;
		goto do_err_resp;
	}
	if((ret = g_pcontainer_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"unlock storage vnode container rwlock failed, err: %s", \
			__LINE__, strerror(ret));
		goto do_err_resp;
	}
	if(storage_vnode_ptr->vnode_status != KL_REPLICA_STATUS_ACTIVE)
	{
		ret = EACCES;
		goto do_err_resp;
	}
	//try to write file to second replica
	snprintf(true_file_path, KL_COMMON_PATH_LEN, "%s/"KL_STORAGE_VNODE_NAME_PREFIX"%d/" \
		"object/%s", \
		g_device_root, nvnode_id, (const char *)(upload_file_ptr->file_path));
	try
	{
		const char *p;
		char dir_path[KL_COMMON_PATH_LEN];

		memset(dir_path, 0, KL_COMMON_PATH_LEN);
		p = strrchr(true_file_path, '/');
		memcpy(dir_path, true_file_path, p - true_file_path);
		if((ret = CDirectory().make_dir(dir_path)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"storage ctrl stream create file dir(dir_path: %s) failed, err: %s", \
				__LINE__, dir_path, strerror(ret));
			goto do_err_resp;
		}

		CFileWriterStream file_writer_stream(true_file_path, presp_stream);
		if((ret = file_writer_stream.stream_writein(nfile_length)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"file writer stream write file data to file(file_path: %s) failed, err: %s", \
				__LINE__, true_file_path, strerror(ret));
			goto do_err_resp;
		}
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory for storage ctrl stream to write file, file_path: %s", \
			__LINE__, true_file_path);
		ret = ENOMEM;
		goto do_err_resp;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"file writer stream write file data to file(file_path: %s) failed, err: %s", \
			__LINE__, true_file_path, strerror(errcode));
		ret = errcode;
		goto do_err_resp;
	}

	g_pcontainer_rwlock->wrlock();
	storage_vnode_ptr->vnode_version++;
	g_pcontainer_rwlock->unlock();
	goto do_succ_resp;

do_err_resp:
	CSERIALIZER::long2buff(0, resp_header.pkg_len);
	resp_header.cmd = KL_STORAGE_CMD_SERVER_RESP;
	resp_header.status = ret;
	if((res = presp_stream->stream_send(&resp_header, sizeof(pkg_header), g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"storage ctrl resp stream send response header failed, err: %s", \
			__LINE__, strerror(res));
	}
	delete pkg_msg_ptr;
	delete presp_stream;
	return ret;
do_succ_resp:
	CSERIALIZER::long2buff(0, resp_header.pkg_len);
	resp_header.cmd = KL_STORAGE_CMD_SERVER_RESP;
	resp_header.status = 0;
	if((ret = presp_stream->stream_send(&resp_header, sizeof(pkg_header), g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"storage ctrl resp stream send response header failed, err: %s", \
			__LINE__, strerror(ret));
	}
	delete pkg_msg_ptr;
	delete presp_stream;
	return ret;
}

int CStorageMsgParser::do_file_hashcode_check(pcheck_file_info file_info_ptr, \
	bool &bsync_flag)
{
	int ret;
	const char *file_path;
	const char *p;

	file_path = (const char *)(file_info_ptr->file_path);
	//KL_SYS_INFOLOG("file_path: %s", file_path);
	p = strrchr(file_path, '.');
	//db file
	if(strcmp(p, ".db") == 0)
	{
		if((ret = do_db_file_check(file_path, (const char *)(file_info_ptr->hash_code), \
			bsync_flag)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"check db file hashcode failed, file_path: %s, err: %s", \
				__LINE__, file_path, strerror(ret));
			return ret;
		}
	}
	else
	{
		if((ret = do_data_file_check(file_path, (const char *)(file_info_ptr->hash_code), \
			bsync_flag)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"check data file hashcode failed, file_path: %s, err: %s", \
				__LINE__, file_path, strerror(ret));
			return ret;
		}
	}
	return 0;
}

int CStorageMsgParser::do_db_file_check(const char *file_path, const char *hashcode, \
	bool &bsync_flag)
{
	int ret;
	char true_file_path[KL_COMMON_PATH_LEN];
	char local_hashcode[KL_COMMON_MD5_HASH_LEN];
	
	memset(true_file_path, 0, KL_COMMON_PATH_LEN);
	snprintf(true_file_path, KL_COMMON_PATH_LEN, "%s/%s", g_device_root, file_path);
	try
	{
		MD5 md5;
		CFile db_file(true_file_path, O_RDONLY);

		if((ret = md5.update(&db_file)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"get file hashcode failed, file_path: %s, err: %s", \
				__LINE__, true_file_path, strerror(ret));
			return ret;
		}
		md5.to_string(local_hashcode, KL_COMMON_MD5_HASH_LEN);
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to check db file hash", \
			__LINE__);
		return ENOMEM;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create db file object failed, file_path: %s, err: %s", \
			__LINE__, file_path, strerror(errcode));
		return errcode;
	}

	if(memcmp(local_hashcode, hashcode, KL_COMMON_MD5_HASH_LEN) == 0)
	{
		bsync_flag = false;
	}
	else
	{
		bsync_flag = true;
	}
	return 0;
}

int CStorageMsgParser::do_data_file_check(const char *file_path, const char *hashcode, \
	bool &bsync_flag)
{
	int ret;
	char db_path[KL_COMMON_PATH_LEN];
	char vnode_name[KL_COMMON_PATH_LEN];
	char local_hashcode[KL_COMMON_MD5_HASH_LEN];
	const char *p;

	p = strchr(file_path, '/');
	memset(vnode_name, 0, KL_COMMON_PATH_LEN);
	memcpy(vnode_name, file_path, p - file_path);
	snprintf(db_path, KL_COMMON_PATH_LEN, "%s/%s/object/hashcode.db", \
		g_device_root, vnode_name);
	try
	{
		CVnodeHashCode vnode_hashcode;
		CTbHashCode tb_hashcode(db_path);

		if(tb_hashcode.query_hashcode_by_filepath(local_hashcode, \
			KL_COMMON_MD5_HASH_LEN, file_path) == -2)
		{
			//record not exist, add it
			strcpy(vnode_hashcode.file_path, file_path);
			memcpy(vnode_hashcode.hash_code, hashcode, KL_COMMON_MD5_HASH_LEN);
			tb_hashcode.insert_record(&vnode_hashcode);
		}
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to do data file check", \
			__LINE__);
		return ENOMEM;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"do data file check failed, file_path: %s, err: %s", \
			__LINE__, file_path, strerror(errcode));
		return errcode;
	}
	catch(const char *perrmsg)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"do data file check failed, file_path: %s, err: %s", \
			__LINE__, file_path, perrmsg);
		return ENOMEM;
	}

	if(memcmp(local_hashcode, hashcode, KL_COMMON_MD5_HASH_LEN) == 0)
	{
		bsync_flag = false;
	}
	else
	{
		bsync_flag = true;
	}
	return 0;
}