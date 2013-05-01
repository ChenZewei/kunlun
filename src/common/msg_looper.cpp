#include <stdio.h>
#include <unistd.h>
#include "log.h"
#include "msg_queue.h"
#include "msg_parser.h"
#include "msg_looper.h"
#include "common_protocol.h"

CMsgLooper::CMsgLooper(CMsgQueue *pmsg_queue, \
	CMsgParser *pmsg_parser) : m_pmsg_queue(pmsg_queue), \
	m_pmsg_parser(pmsg_parser), m_stop_flag(false)
{
}

CMsgLooper::~CMsgLooper()
{
}

int CMsgLooper::run()
{
	pkg_message *pkg_msg_ptr;
	if(m_pmsg_parser == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"m_pmsg_manager is null", \
			__LINE__);
		return -1;
	}

	if(m_pmsg_queue == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"m_pmsg_queue is null", \
			__LINE__);
		return -1;
	}

	m_stop_flag = false;
	while(m_stop_flag != true)
	{
		if((pkg_msg_ptr = m_pmsg_queue->get_msg()) == NULL)
		{
			KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
				"msg looper get null message", \
				__LINE__);
			continue;
		}

		if(m_pmsg_parser->parse_msg(pkg_msg_ptr) != 0)
		{
			KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
				"msg parser parse msg failed", \
				__LINE__);
			continue;
		}
	}

	return 0; //actually, it will not be executed
}

int CMsgLooper::stop()
{
	m_stop_flag = true;
	return 0;
}