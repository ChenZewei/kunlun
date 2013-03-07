#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

#define KL_BUF_SIZE		256
#define KL_EXIT_ERR		-1 //system exit with error
#define KL_EXIT_SYS		0 //system exit without error

#include <unistd.h>

static void errout(const char *str)
{
	perror(str);
	_exit(KL_EXIT_ERR);
}

#endif //COMMON_TYPES_H_