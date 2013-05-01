#ifndef KL_PROXY_SERVER_CONF_H_
#define KL_PROXY_SERVER_CONF_H_
#include "base_server_conf.h"
class CProxyServerConf : public CBaseServerConf
{
public:
	int nvnode_count;
	int nreplica_count;
	bool bmaster_flag;
	int nnamespace_power;
};
#endif //KL_PROXY_SERVER_CONF_H_