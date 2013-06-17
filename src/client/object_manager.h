#ifndef KL_CLIENT_OBJECT_MANAGER_H_
#define KL_CLIENT_OBJECT_MANAGER_H_
#include "timed_stream.h"
class CObjectManager
{
public:
	CObjectManager();
	~CObjectManager();

	int upload_object_file_by_buff(CTimedStream *pproxy_stream, const char *account_name, \
		const char *container_name, const char *object_name, const char *file_buf, \
		int nlength, int ntimeout);
private:
	int get_proxy_list(CTimedStream *pproxy_stream, const char *file_path, int *pvnode_id, \
		pstorage_info *ppstorage_addr_list, int *list_count, int ntimeout);
};
#endif //KL_CLIENT_OBJECT_MANAGER_H_