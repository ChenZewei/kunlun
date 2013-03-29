#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "socknotifier.h"
#include "sockobserver.h"
#include "epoll_engine.h"
#include "acceptorOB.h"

int main(int argc, char *argv[])
{
	if(argc != 2){
		printf("error args, Usage %s <port>\n", argv[0]);
		return -1;
	}

	CEpollEngine *pEpEngine;
	CAcceptorOB *pAcceptorOB;
	int port;
	
	port = atoi(argv[1]);
	if(port < 0){
		return -1;
	}
	pEpEngine = new CEpollEngine();
	pAcceptorOB = new CAcceptorOB(port, 1024);
	pEpEngine->attach(pAcceptorOB, EPOLLIN | EPOLLET);
	pEpEngine->run();
	pEpEngine->detach(pAcceptorOB);
	delete pEpEngine;
	delete pAcceptorOB;
	return 0;
}
