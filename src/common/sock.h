/*******************************************
* Author: Leslie Wei
* Created Time: 2012年08月11日 星期六 23时06分53秒
* File Name: sock.h
* Description: 
* @Copyright reserved
********************************************/
#ifndef _SOCK_H_
#define _SOCK_H_
#include "inetaddr.h"
class CSock
{
public:
	int open(int domain, int type);
	int getSocket() const;
	int getLocalAddr(CInetAddr *pAddr);
	int getPeerAddr(CInetAddr *pAddr);
	void setNonBlocking();
	virtual ~CSock();
protected:
	CSock();
	CSock(int domain, int type);

	int m_fd;
};
#endif //_SOCK_H_
