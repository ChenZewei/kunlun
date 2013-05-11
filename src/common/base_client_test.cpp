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
#include "log.h"
#include "timed_stream.h"
#include "common_protocol.h"

#define gettid() syscall(SYS_gettid)

#define KL_COMMON_PROTOCOL_MSG_TEST1 0
//#define KL_COMMON_PROTOCOL_MSG_TEST2 1
//#define KL_COMMON_PROTOCOL_MSG_TEST3 2
#define KL_COMMON_PROTOCOL_MSG_SERVER_RESP 3
typedef pkg_header base_msg_header;

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
	g_psys_log = new CLog("./base_client_test.log", LOG_LEVEL_DEBUG);
	for(i = 0; i < 20; i++)
	{
		ThreadArgs *pta = new ThreadArgs();
		pta->port = port;
		strcpy(pta->host, argv[1]);
		if(pthread_create(&pid[i], NULL, client_process, pta) == -1)
		{
			perror("pthread_create error");
			return -1;
		}
	}

	for(i = 0; i < 20; i++)
	{
		pthread_join(pid[i], NULL);
	}
	pthread_mutex_destroy(&io_lock);
	return 0;
}

void* client_process(void *args)
{
	int i, ret;
	int64_t pkg_len;
	int sock_fd;
	char inbuf[100];
	char outbuf[100];
	struct sockaddr_in serverAddr;

	ThreadArgs *pta = (ThreadArgs*)args;
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1)
	{
		delete pta;
		return NULL;
	}
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(pta->port);
	inet_aton(pta->host, &serverAddr.sin_addr);
	if(connect(sock_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) \
			== -1)
	{
		perror("connect error");
		delete pta;
		return NULL;
	}
	
	sprintf(outbuf, "client(thread: %ld) send test1 data", gettid());
	base_msg_header base_header;
	CTimedStream timed_stream(sock_fd);
	for(i = 0; i < 20; i++)
	{
		base_header.cmd = KL_COMMON_PROTOCOL_MSG_TEST1;
		base_header.status = 0;
		CSERIALIZER::long2buff(strlen(outbuf), base_header.pkg_len);
		/*send(sock_fd, &base_header, sizeof(base_msg_header), 0);
		send(sock_fd, outbuf, strlen(outbuf), 0);*/
		if((ret = timed_stream.stream_send(&base_header, sizeof(base_header), 5)) != 0)
		{
			printf("file: "__FILE__", line: %d, " \
				"timed stream send base header failed, err: %s\n", \
				__LINE__, strerror(ret));
			delete pta;
			return NULL;
		}
		if((ret = timed_stream.stream_send(outbuf, strlen(outbuf), 5)) != 0)
		{
			printf("file: "__FILE__", line: %d, " \
				"timed stream send base msg body failed, err: %s\n", \
				__LINE__, strerror(ret));
			delete pta;
			return NULL;
		}
		pthread_mutex_lock(&io_lock);
		printf("client(thread: %ld) send data to server\n", gettid());
		pthread_mutex_unlock(&io_lock);

		/*recv(sock_fd, &base_header, sizeof(base_msg_header), 0);
		recv(sock_fd, inbuf, sizeof(inbuf), 0);*/
		if((ret = timed_stream.stream_recv(&base_header, sizeof(base_header), 1)) != 0)
		{
			printf("file: "__FILE__", line: %d, " \
				"timed stream recv base msg header failed, err: %s\n", \
				__LINE__, strerror(ret));
			delete pta;
			return NULL;
		}
		memset(inbuf, 0, sizeof(inbuf));
		pkg_len = CSERIALIZER::buff2int64(base_header.pkg_len);
		if((ret = timed_stream.stream_recv(inbuf, pkg_len, 1)) != 0)
		{
			printf("file: "__FILE__", line: %d, " \
				"timed stream recv base msg body failed, err: %s\n", \
				__LINE__, strerror(ret));
			delete pta;
			return NULL;
		}
		pthread_mutex_lock(&io_lock);
		printf("client(thread: %ld) recv data from server: %s\n", gettid(), inbuf);
		pthread_mutex_unlock(&io_lock);
	}
	//close(sock_fd);
	delete pta;
	return NULL;
}
