/*******************************************
* Author: Leslie Wei
* Created Time: 2012年08月14日 星期二 16时13分55秒
* File Name: epollserver.cpp
* Description: 
* @Copyright reserved
********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "subject.h"
#include "observer.h"
#include "epengine.h"
#include "acceptorOB.h"

int main(int argc, char *argv[])
{
	if(argc != 2){
		printf("error args, Usage %s <port>\n", argv[0]);
		return -1;
	}

	CEpEngine *pEpEngine;
	CAcceptorOB *pAcceptorOB;
	int port;
	
	port = atoi(argv[1]);
	if(port < 0){
		return -1;
	}
	pEpEngine = new CEpEngine();
	pAcceptorOB = new CAcceptorOB(port, 1024);
	pEpEngine->Attach(pAcceptorOB, EPOLLIN | EPOLLET);
	pEpEngine->Run();
	pEpEngine->Detach(pAcceptorOB);
	delete pEpEngine;
	delete pAcceptorOB;
	return 0;
}
