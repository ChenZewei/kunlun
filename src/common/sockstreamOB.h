#ifndef _SOCK_STREAM_OB_H_
#define _SOCK_STREAM_OB_H_
#include "sockstream.h"
#include "observer.h"
class CSockStreamOB : public CSockStream, public CObserver
{
public:
	CSockStreamOB();
	CSockStreamOB(int sock);

	int GetFd() const;
	void Work(CSubject *pSubject, uint32_t nstatus);
};
#endif //_SOCK_STREAM_OB_H_
