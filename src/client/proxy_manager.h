#ifndef KL_CLIENT_PROXY_MANAGER_H_
#define KL_CLIENT_PROXY_MANAGER_H_
#include "inetaddr.h"
#include "timed_stream.h"
class CProxyServerManager
{
public:
	CProxyServerManager(CInetAddr *proxy_addr_list, int nlist_count);
	~CProxyServerManager();

	CTimedStream *get_proxy_stream_by_robin();
private:
	int m_proxy_stream_robin;
	int m_proxy_stream_count;
	CTimedStream **m_proxy_stream_container;
};
#endif //KL_CLIENT_PROXY_MANAGER_H_