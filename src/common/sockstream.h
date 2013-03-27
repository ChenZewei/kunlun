/*******************************************
* Author: Leslie Wei
* Created Time: 2012年08月13日 星期一 22时18分48秒
* File Name: sockstream.h
* Description: CSockStream类继承于CSock类，用于封装建立TCP连接的数据流收发操作
* @Copyright reserved
********************************************/
#ifndef _SOCK_STREAM_H_
#define _SOCK_STREAM_H_

#include "sock.h"

class CSockStream : public CSock
{
public:
	CSockStream();
	CSockStream(int sock);
	
	void SetSockStream(int sock);
	int Send(const void *buf, size_t len);
	int Recv(void *buf, size_t len);
};
#endif //_SOCK_STREAM_H_
