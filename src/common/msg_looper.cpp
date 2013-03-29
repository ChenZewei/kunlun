#include <stdio.h>
#include <unistd.h>
#include "log.h"
#include "msg_queue.h"
#include "msg_manager.h"
#include "msg_looper.h"
#include "common_protocol.h"

CMsgLooper::CMsgLooper(CMsgQueue *pmsg_queue, \
	CMsgManager *pmsg_manager) : m_pmsg_queue(pmsg_queue), \
	m_pmsg_manager(pmsg_manager)
{
}

CMsgLooper::~CMsgLooper()
{
	if(m_pmsg_manager != NULL)
	{
		delete m_pmsg_manager;
		m_pmsg_manager = NULL;
	}
}

int CMsgLooper::run()
{
	pkg_message *pkg_msg_ptr;
	if(m_pmsg_manager == NULL)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"m_pmsg_manager is null", \
			__LINE__);
		return -1;
	}

	if(m_pmsg_queue == NULL)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"m_pmsg_queue is null", \
			__LINE__);
		return -1;
	}

	while(true){
		if((pkg_msg_ptr = m_pmsg_queue->get_msg()) == NULL)
		{
			KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
				"msg looper get null message", \
				__LINE__);
			continue;
		}

		if(m_pmsg_manager->despatch_msg(pkg_msg_ptr) != 0)
		{
			KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
				"msg manager despatch msg failed", \
				__LINE__);
			return -1;
		}
	}

	return 0; //actually, it will not be executed
}