#ifndef KL_STORAGE_GLOBAL_H_
#define KL_STORAGE_GLOBAL_H_
#include "common_types.h"
class CLog;
class CRWLock;
class CStorageVnodeContainer;
extern int g_timeout;
extern char g_storage_bind_host[KL_COMMON_IP_ADDR_LEN];
extern int g_storage_bind_port;
extern CLog *g_psys_log;
extern CRWLock *g_pcontainer_rwlock;
extern CStorageVnodeContainer *g_pstorage_vnode_container;

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus
	void destroy_global_data();
#ifdef __cplusplus
}
#endif //__cplusplus
#endif //KL_STORAGE_GLOBAL_H_