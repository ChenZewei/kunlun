#ifndef KL_COMMON_SOCK_H_
#define KL_COMMON_SOCK_H_
#include "inetaddr.h"
class CSock
{
public:
	int open(int domain, int type);
	int getSocket() const;
	int getLocalAddr(CInetAddr *pAddr);
	int getPeerAddr(CInetAddr *pAddr);
	void setnonblocking();
	virtual ~CSock();
protected:
	CSock();
	CSock(int domain, int type);

	int m_fd;
};
#endif //KL_COMMON_SOCK_H_
