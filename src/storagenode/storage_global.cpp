#include <stdio.h>
#include "log.h"
#include "rwlock.h"
#include "vnode_info.h"
#include "storage_global.h"

int g_timeout = 0;
int g_storage_bind_port = 0;
char g_storage_bind_host[KL_COMMON_IP_ADDR_LEN] = {'\0'};
CRWLock *g_pcontainer_rwlock = NULL;
CStorageVnodeContainer *g_pstorage_vnode_container = NULL;

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus
	void destroy_global_data()
	{
		if(g_pcontainer_rwlock != NULL)
		{
			delete g_pcontainer_rwlock;
			g_pcontainer_rwlock = NULL;
		}

		if(g_pstorage_vnode_container != NULL)
		{
			delete g_pstorage_vnode_container;
			g_pstorage_vnode_container = NULL;
		}

		if(g_psys_log != NULL)
		{
			delete g_psys_log;
			g_psys_log = NULL;
		}
	}
#ifdef __cplusplus
}
#endif //__cplusplus