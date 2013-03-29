#ifndef KL_COMMON_SOCK_STREAM_OB_H_
#define KL_COMMON_SOCK_STREAM_OB_H_
#include "sockstream.h"
#include "sockobserver.h"
class CMsgQueue;
class CSockStreamOB : public CSockStream, \
	public CSockObserver
{
public:
	CSockStreamOB();
	CSockStreamOB(int sock);
	virtual ~CSockStreamOB();

	int get_fd() const;
	virtual void work(CSockNotifier *psock_notifier, uint32_t nstatus);
};
#endif //KL_COMMON_SOCK_STREAM_OB_H_
