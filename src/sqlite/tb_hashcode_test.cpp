#include <new>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "tb_hashcode.h"

int main(int argc, char *argv[])
{
	CTbHashCode *ptb_hashcode;

	try
	{
		g_psys_log = new CLog("./tb_hashcode_test.log", LOG_LEVEL_DEBUG);
	}
	catch(std::bad_alloc)
	{
		g_psys_log = NULL;
		printf("no more memory to create sys log\n");
		return ENOMEM;
	}
	catch(int errcode)
	{
		g_psys_log = NULL;
		printf("create sys log failed, err: %s\n", strerror(errcode));
		return errcode;
	}

	try
	{
		ptb_hashcode = new CTbHashCode("./hashcode_test.db");
	}
	catch(std::bad_alloc)
	{
		ptb_hashcode = NULL;
		printf("no more memory to create tb_hashcode\n");
		return ENOMEM;
	}
	catch(int errcode)
	{
		ptb_hashcode = NULL;
		printf("create ptb_hashcode failed, err: %s\n", strerror(errcode));
		return errcode;
	}
	catch(const char *perrmsg)
	{
		ptb_hashcode = NULL;
		printf("create ptb_hashcode failed, err: %s\n", perrmsg);
		return -1;
	}

	delete ptb_hashcode;
	try
	{
		ptb_hashcode = new CTbHashCode("./hashcode_test.db");
	}
	catch(std::bad_alloc)
	{
		ptb_hashcode = NULL;
		printf("no more memory to create tb_hashcode\n");
		return ENOMEM;
	}
	catch(int errcode)
	{
		ptb_hashcode = NULL;
		printf("create ptb_hashcode failed, err: %s\n", strerror(errcode));
		return errcode;
	}
	catch(const char *perrmsg)
	{
		ptb_hashcode = NULL;
		printf("create ptb_hashcode failed, err: %s\n", perrmsg);
		return -1;
	}
	delete ptb_hashcode;
	delete g_psys_log;
	return 0;
}