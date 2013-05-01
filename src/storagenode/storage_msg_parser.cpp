#include <stdio.h>
#include <unistd.h>
#include "log.h"
#include "storage_msg_parser.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CStorageMsgParser::CStorageMsgParser()
{
}

CStorageMsgParser::~CStorageMsgParser()
{
}

int CStorageMsgParser::parse_msg(pkg_message* pkg_msg_ptr)
{
#ifdef _DEBUG
	assert(pkg_msg_ptr);
#endif //_DEBUG
	byte msg_cmd;
	if(pkg_msg_ptr->pkg_ptr == NULL)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"the context of message package is null", \
			__LINE__);
		return -1;
	}

	msg_cmd = *(pkg_msg_ptr->pkg_ptr);
	switch (msg_cmd)
	{
	}
	return -1;
}