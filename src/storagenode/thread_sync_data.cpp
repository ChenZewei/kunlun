#include <new>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "thread_sync_data.h"

CThreadSyncData::CThreadSyncData(CMsgQueue *psync_msg_queue) : \
	m_psync_msg_queue(psync_msg_queue)
{

}

CThreadSyncData::~CThreadSyncData()
{

}

int CThreadSyncData::run()
{

}

int CThreadSyncData::stop()
{

}