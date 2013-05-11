#ifndef KL_STORAGE_SERVER_H_
#define KL_STORAGE_SERVER_H_
#include "base_server.h"
#include "storage_msg_parser.h"
#include "storage_server_conf.h"
class CThread;
class CMsgQueue;
class CStorageServer : public CBaseServer
{
public:
	CStorageServer(CStorageServerConf storage_server_conf, \
		CStorageMsgParser *pstorage_msg_parser);
	virtual ~CStorageServer();

	int initilize();
	virtual int run();
	int stop();
private:
	int join_and_report();

	CThread **m_ppreport_threads;
	CThread *m_psync_thread;
	CMsgQueue *m_psync_msg_queue;
	CStorageServerConf m_storage_server_conf;
};
#endif //KL_STORAGE_SERVER_H_