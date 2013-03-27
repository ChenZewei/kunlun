/*******************************************
* Author: Leslie Wei
* Created Time: 2012年08月14日 星期二 17时57分07秒
* File Name: epollclient.cpp
* Description: 
* @Copyright reserved
********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define gettid() syscall(SYS_gettid)

pthread_mutex_t io_lock;
void* client_process(void *args);
typedef struct thread_args
{
	int port;
	char host[20];
}ThreadArgs;

int main(int argc, char *argv[])
{
	int port;
	int i;
	pthread_t pid[20];
	
	if(argc != 3){
		printf("error args, Usage %s <host> <port>\n", argv[0]);
		return -1;
	}

	port = atoi(argv[2]);
	if(port < 0){
		return -1;
	}

	pthread_mutex_init(&io_lock, NULL);
	for(i = 0; i < 20; i++){
		ThreadArgs *pta = new ThreadArgs();
		pta->port = port;
		strcpy(pta->host, argv[1]);
		if(pthread_create(&pid[i], NULL, client_process, pta) == -1){
			perror("pthread_create error");
			return -1;
		}
	}

	for(i = 0; i < 20; i++){
		pthread_join(pid[i], NULL);
	}
	pthread_mutex_destroy(&io_lock);
	return 0;
}

void* client_process(void *args)
{
	int i;
	int sock_fd;
	char inbuf[100];
	char outbuf[100];
	struct sockaddr_in serverAddr;

	ThreadArgs *pta = (ThreadArgs*)args;
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1){
		delete pta;
		return NULL;
	}
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(pta->port);
	inet_aton(pta->host, &serverAddr.sin_addr);
	if(connect(sock_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) \
			== -1){
		perror("connect error");
		delete pta;
		return NULL;
	}
	
	sprintf(outbuf, "client %d send test data", gettid());
	for(i = 0; i < 20; i++){
		send(sock_fd, outbuf, strlen(outbuf) + 1, 0);
		pthread_mutex_lock(&io_lock);
		printf("%d send data to server\n", gettid());
		pthread_mutex_unlock(&io_lock);

		recv(sock_fd, inbuf, sizeof(inbuf), 0);
		pthread_mutex_lock(&io_lock);
		printf("%d recv data from server: %s\n", gettid(), inbuf);
		pthread_mutex_unlock(&io_lock);
	}
	delete pta;
	return NULL;
}
