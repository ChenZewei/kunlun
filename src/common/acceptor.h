/*******************************************
* Author: Leslie Wei
* Created Time: 2012年08月13日 星期一 21时56分15秒
* File Name: acceptor.h
* Description: 
* @Copyright reserved
********************************************/
#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include "sock.h"
#include "sockstream.h"
#include "inetaddr.h"
class CAcceptor : public CSock
{
public:
	//port用于指定监听者的端口
	//backlog 用于指定最大的监听排队数
	CAcceptor(int port, int backlog);
	CAcceptor(CInetAddr& sockAddr, int backlog);

	int Accept(CSockStream *pSockStream);
};
#endif //_ACCEPTOR_H_
