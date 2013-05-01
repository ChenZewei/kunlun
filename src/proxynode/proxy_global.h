#ifndef KL_PROXY_GLOBAL_H_
#define KL_PROXY_GLOBAL_H_

class CVnodeContainer;
class CDeviceContainer;
class CRWLock;
class CLog;
extern CLog *g_psys_log;
extern CDeviceContainer *g_pdevice_container;
extern CVnodeContainer *g_pvnode_container;
extern int g_namespace_power;
extern bool g_master_flag;
extern int g_timeout;
extern CRWLock *g_pdevice_chg_rwlock;

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus
	void destroy_global_data();
#ifdef __cplusplus
}
#endif //__cplusplus
#endif //KL_PROXY_GLOBAL_H_