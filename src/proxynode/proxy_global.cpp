#include <stdio.h>
#include "log.h"
#include "hash.h"
#include "rwlock.h"
#include "node_container.h"
#include "proxy_global.h"

CDeviceContainer *g_pdevice_container = NULL;
CVnodeContainer *g_pvnode_container = NULL;
//int g_namespace_power = 0;
bool g_bmaster_flag = false;
int g_ntimeout = 0;
CRWLock *g_pdevice_chg_rwlock = NULL;
CNameSpaceHash *g_pnamespace_hash = NULL;

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus
	void destroy_global_data()
	{
		if(g_pdevice_container != NULL)
		{
			delete g_pdevice_container;
			g_pdevice_container = NULL;
		}

		if(g_pvnode_container != NULL)
		{
			delete g_pvnode_container;
			g_pvnode_container = NULL;
		}

		if(g_pdevice_chg_rwlock != NULL)
		{
			delete g_pdevice_chg_rwlock;
			g_pdevice_chg_rwlock = NULL;
		}

		if(g_psys_log != NULL)
		{
			delete g_psys_log;
			g_psys_log = NULL;
		}

		if(g_pnamespace_hash != NULL)
		{
			delete g_pnamespace_hash;
			g_pnamespace_hash = NULL;
		}
	}
#ifdef __cplusplus
}
#endif //__cplusplus