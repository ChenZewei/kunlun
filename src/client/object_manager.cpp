#include <new>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "connector.h"
#include "proxy_protocol.h"
#include "object_manager.h"
#include "storage_protocol.h"

CObjectManager::CObjectManager()
{

}

CObjectManager::~CObjectManager()
{

}

int CObjectManager::get_proxy_list(CTimedStream *pproxy_stream, const char *file_path, \
	int *pvnode_id, pstorage_info *ppstorage_addr_list, int *list_count, int ntimeout)
{
	int ret, i;
	byte *pbody;
	int64_t pkg_len;
	int nstorage_port;
	int nstorage_addr_count;
	pkg_header proxy_header;
	file_info object_file_info;
	pstorage_info storage_info_ptr;

	CSERIALIZER::long2buff(sizeof(file_info), proxy_header.pkg_len);
	proxy_header.cmd = KL_PROXY_CMD_WRITE_FILE;
	proxy_header.status = 0;
	memset(&object_file_info, 0, sizeof(file_info));
	strcpy((char *)(object_file_info.file_path), file_path);

	*ppstorage_addr_list = NULL;
	*list_count = 0;
	*pvnode_id = -1;
	if((ret = pproxy_stream->stream_send(&proxy_header, sizeof(pkg_header), ntimeout)) != 0)
	{
		return ret;
	}
	if((ret = pproxy_stream->stream_send(&object_file_info, sizeof(file_info), ntimeout)) != 0)
	{
		return ret;
	}
	
	if((ret = pproxy_stream->stream_recv(&proxy_header, sizeof(pkg_header), ntimeout)) != 0)
	{
		return ret;
	}
	if(proxy_header.status != 0)
	{
		return proxy_header.status;
	}
	
	pkg_len = CSERIALIZER::buff2int64(proxy_header.pkg_len);
	if((pkg_len - 4) % sizeof(storage_info) != 0)
	{
		return EINVAL;
	}

	try
	{
		pbody = new byte[pkg_len];
	}
	catch(std::bad_alloc)
	{
		return ENOMEM;
	}
	catch(int errcode)
	{
		return errcode;
	}

	if((ret = pproxy_stream->stream_recv(pbody, pkg_len, ntimeout)) != 0)
	{
		return ret;
	}

	*pvnode_id = CSERIALIZER::buff2int32(pbody);
	nstorage_addr_count = (pkg_len - 4) / sizeof(storage_info);
	try
	{
		storage_info_ptr = new storage_info[nstorage_addr_count];
	}
	catch(std::bad_alloc)
	{
		delete [] pbody;
		return ENOMEM;
	}
	memcpy(storage_info_ptr, (pbody + 4), pkg_len - 4);
	*ppstorage_addr_list = storage_info_ptr;
	*list_count = nstorage_addr_count;
	return 0;
}

int CObjectManager::upload_object_file_by_buff(CTimedStream *pproxy_stream, \
	const char *account_name, const char *container_name, const char *object_name, \
	const char *file_buf, int nlength, int ntimeout)
{
	int ret;
	int nlist_count;
	int nvnode_id;
	int nstorage_port;
	int64_t npkg_len;
	pkg_header primary_header;
	upload_file_info upload_file;
	CTimedStream *pstorage_stream;
	CInetAddr primary_storage_addr;
	pstorage_info storage_info_ptr;
	char file_path[KL_COMMON_PATH_LEN];

	memset(file_path, 0, KL_COMMON_PATH_LEN);
	snprintf(file_path, KL_COMMON_PATH_LEN, "%s/%s/%s", \
		account_name, container_name, object_name);

	if((ret = get_proxy_list(pproxy_stream, file_path, &nvnode_id, &storage_info_ptr, \
		&nlist_count, ntimeout)) != 0)
	{
		return ret;
	}
	primary_storage_addr.setsockaddr((const char *)(storage_info_ptr->device_bind_ip), \
		nstorage_port);
	try
	{
		CConnector storage_connector(primary_storage_addr);
		pstorage_stream = new CTimedStream();

		if((ret = storage_connector.stream_connect(pstorage_stream)) != 0)
		{
			delete [] storage_info_ptr;
			return ret;
		}
	}
	catch(std::bad_alloc)
	{
		delete [] storage_info_ptr;
		return ENOMEM;
	}
	catch(int errcode)
	{
		delete [] storage_info_ptr;
		return errcode;
	}
	
	//do file upload
	npkg_len = sizeof(upload_file_info) + sizeof(storage_info) * (nlist_count - 1);
	CSERIALIZER::long2buff(npkg_len, primary_header.pkg_len);
	primary_header.cmd = KL_STORAGE_CMD_UPLOAD_FILE;
	primary_header.status = 0;
	if((ret = pstorage_stream->stream_send(&primary_header, sizeof(pkg_header), ntimeout)) != 0)
	{
		delete [] storage_info_ptr;
		return ret;
	}
	CSERIALIZER::int2buff(nvnode_id, upload_file.vnode_id);
	CSERIALIZER::long2buff(nlength, upload_file.file_length);
	memcpy(upload_file.file_path, file_path, KL_COMMON_PATH_LEN);
	if((ret = pstorage_stream->stream_send(&upload_file, sizeof(upload_file_info), \
		ntimeout)) != 0)
	{
		delete [] storage_info_ptr;
		return ret;
	}
	if((ret = pstorage_stream->stream_send((const void *)(storage_info_ptr + 1), \
		sizeof(storage_info) * (nlist_count - 1), ntimeout)) != 0)
	{
		delete [] storage_info_ptr;
		return ret;
	}
	if((ret = pstorage_stream->stream_send(file_buf, nlength, ntimeout)) != 0)
	{
		delete [] storage_info_ptr;
		return ret;
	}

	if((ret = pstorage_stream->stream_recv(&primary_header, sizeof(pkg_header), ntimeout)) != 0)
	{
		delete [] storage_info_ptr;
		return ret;
	}
	
	return primary_header.status;
}