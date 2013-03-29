#ifndef KL_COMMON_EVENT_H_
#define KL_COMMON_EVENT_H_
#include "mutex.h"
#include "cond.h"
class CEvent
{
public:
	CEvent();
	CEvent(bool bsemaphore);
	virtual ~CEvent();

	int signal_set();
	int signal_wait();
private:
	CEvent(const CEvent&);
	CEvent& operator=(const CEvent&);
	
	CMutex m_signal_mutex;
	CCond m_signal_cond;
	int m_nsignal;
	bool m_bsemaphore;
};

#endif //KL_COMMON_EVENT_H_