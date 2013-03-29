#ifndef KL_COMMON_CONNECTOR_H_
#define KL_COMMON_CONNECTOR_H_
#include "sock.h"
#include "inetaddr.h"
#include "sockstream.h"
class CConnector : public CSock
{
public:
	//host:要连接的服务端的地址名
	//port:要连接的服务端的端口名
	//bport:可选参数,指定绑定到本地的端口
	CConnector(const char *host, int port, int bport = -1);
	//serverAddr:服务端的地址
	CConnector(const CInetAddr& serverAddr, int bport = -1);

	//pSockStream:用于指定一个TCP流，如果连接成功，将初始化一个TCP流
	int Connect(CSockStream *pSockStream);
protected:
	CInetAddr m_serveraddr;
};
#endif //KL_COMMON_CONNECTOR_H_
