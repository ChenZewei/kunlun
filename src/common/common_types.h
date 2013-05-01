#ifndef KL_COMMON_TYPES_H_
#define KL_COMMON_TYPES_H_

/*
 * @description: set buf size to 4096 just because the size is similar to
                 the size of unix kernel's buffer, even though set buf to 
				 more size, the efficiency of copy isn't promoted
 */
#define KL_COMMON_BUF_SIZE		4096 
#define KL_COMMON_IP_ADDR_LEN	16	//IPv4
#define KL_COMMON_EXIT_ERR		-1	//system exit with error
#define KL_COMMON_EXIT_SYS		0	//system exit without error

#ifdef USE_SELECT
#define KL_COMMON_STREAM_IN
#define KL_COMMON_STREAM_OUT
#define KL_COMMON_STREAM_EXTEND	0
#else
#ifdef USE_POLL
#define KL_COMMON_STREAM_IN
#define KL_COMMON_STREAM_OUT
#define KL_COMMON_STREAM_EXTEND	0
#else //default : USE_EPOLL
#define KL_COMMON_STREAM_IN EPOLLIN
#define KL_COMMON_STREAM_OUT EPOLLOUT
#define KL_COMMON_STREAM_EXTEND	EPOLLET
#endif //USE_POLL
#endif //USE_SELECT

#include <new>
#define kl_new new(std::nothrow)

#include <stdio.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	static void kl_errout(const char *str)
	{
		printf("%s\n", str);
		_exit(KL_COMMON_EXIT_ERR);
	}

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //KL_COMMON_TYPES_H_