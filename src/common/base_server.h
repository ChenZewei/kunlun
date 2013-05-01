#ifndef KL_COMMON_BASE_SERVER_H_
#define KL_COMMON_BASE_SERVER_H_
#include "base_server_conf.h"
class CThread;
class CMsgQueueArr;
class CMsgParser;
class CBaseServer
{
public:
	/*
	 * @param: pmsg_parser, the object must creat on heap, and delete by base server
	 */
	CBaseServer(CBaseServerConf base_server_conf, CMsgParser *pmsg_parser);
	virtual ~CBaseServer();

	int initilize();
	virtual int run();
	int stop();
protected:
	int m_work_thread_count;
	CBaseServerConf m_base_server_conf;
	//a msg recv thread and m_work_thread_count work threads
	CThread **m_ppthread;
	CMsgQueueArr *m_pmsg_queue_arr;
	CMsgParser *m_pmsg_parser;
};
#endif //KL_COMMON_BASE_SERVER_H_