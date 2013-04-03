#ifndef KL_COMMON_CONNECTOR_H_
#define KL_COMMON_CONNECTOR_H_
#include "sock.h"
#include "inetaddr.h"
#include "sockstream.h"
/*
 * @description: connector is a sock stream factory, if connected 
				 successfully, connector will initilize a sock stream
 */
class CConnector : public CSock
{
public:
	CConnector(const char *host, int port, int bport = -1);
	CConnector(const CInetAddr& serverAddr, int bport = -1);
	~CConnector();

	int stream_connect(CSockStream *pSockStream);
protected:
	CInetAddr m_serveraddr;
	bool m_isconnected;
};
#endif //KL_COMMON_CONNECTOR_H_
