#ifndef KL_STORAGE_GLOBAL_H_
#define KL_STORAGE_GLOBAL_H_
#include <queue>
#include "common_types.h"

#define KL_STORAGE_BEAT_INTERVAL 5
#define KL_STORAGE_SYNC_REPORT_INTERVAL 5
#define KL_STORAGE_VNODE_NAME_PREFIX "storage_vnode"

class CLog;
class CRWLock;
class CStorageVnodeContainer;
extern int g_ntimeout;
extern int g_storage_beat_interval;
extern char g_storage_bind_ip[KL_COMMON_IP_ADDR_LEN];
extern char g_device_root[KL_COMMON_PATH_LEN];
extern int g_nstorage_bind_port;
extern int g_nstorage_zone_id;
extern int g_nstorage_weight;
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