/*******************************************
* Author: Leslie Wei
* Created Time: 2012年08月14日 星期二 10时01分29秒
* File Name: acceptorOB.h
* Description: 
* @Copyright reserved
********************************************/
#ifndef _ACCEPTOR_OB_H_
#define _ACCEPTOR_OB_H_
#include "acceptor.h"
#include "observer.h"
#include "inetaddr.h"
class CAcceptorOB : public CAcceptor, public CObserver
{
public:
	CAcceptorOB(int port, int backlog);
	CAcceptorOB(CInetAddr& sockAddr, int backlog);

	void Work(CSubject *pSubject, uint32_t nstatus);
	int GetFd() const;
};
#endif //_ACCEPTOR_OB_H_
