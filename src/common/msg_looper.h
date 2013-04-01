#ifndef KL_COMMON_MSG_LOOPER_H_
#define KL_COMMON_MSG_LOOPER_H_
#include "thread_func.h"
class CMsgQueue;
class CMsgParser;
class CMsgLooper : public CThreadFunc
{
public:
	/*
	 * @param: pmsg_queue, msg looper get msg from the queue
	 * @param: pmsg_parser, set a parser to parse msg, msg parser
			   is shared by every work thread, can't be deleted in msg looper
	 */
	CMsgLooper(CMsgQueue *pmsg_queue, CMsgParser *pmsg_parser);
	~CMsgLooper();

	virtual int run();
private:
	CMsgQueue *m_pmsg_queue;
	CMsgParser *m_pmsg_parser;
};

#endif //KL_COMMON_MSG_LOOPER_H_