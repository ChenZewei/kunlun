#ifndef KL_COMMON_TYPES_H_
#define KL_COMMON_TYPES_H_

#define KL_COMMON_BUF_SIZE		256
#define KL_COMMON_EXIT_ERR		-1	//system exit with error
#define KL_COMMON_EXIT_SYS		0	//system exit without error

#include <stdio.h>
#include <unistd.h>

static void kl_errout(const char *str)
{
	/*
	char msgbuf[KL_COMMON_BUF_SIZE];

	memset(msgbuf, 0, KL_COMMON_BUF_SIZE);

	snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
		"%s, err: %s\n", \
		str, strerror(errno));
	*/
	printf("%s\n", str);
	_exit(KL_COMMON_EXIT_ERR);
}

#endif //KL_COMMON_TYPES_H_