#ifndef KL_COMMON_SOCK_H_
#define KL_COMMON_SOCK_H_
#include "inetaddr.h"
class CSock
{
public:
	int open(int domain, int type);
	int getsocket() const;
	int getlocaladdr(CInetAddr *pAddr);
	int getpeeraddr(CInetAddr *pAddr);
	int setnonblocking();
	int setblocking();
	virtual ~CSock();
protected:
	CSock();
	CSock(int domain, int type);

	int m_fd;
};
#endif //KL_COMMON_SOCK_H_
