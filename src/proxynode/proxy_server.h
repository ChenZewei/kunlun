#ifndef KL_PROXY_SERVER_H_
#define KL_PROXY_SERVER_H_
#include "base_server.h"
#include "proxy_server_conf.h"
class CProxyMsgParser;
class CProxyServer : public CBaseServer
{
public:
	CProxyServer(CProxyServerConf proxy_server_conf, \
		CProxyMsgParser *pproxy_msg_parser);
	~CProxyServer();

	int initilize();
	int run();
	int stop();
private:
	CProxyServerConf m_proxy_server_conf;
};
#endif //KL_PROXY_SERVER_H_