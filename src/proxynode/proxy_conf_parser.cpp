#include <stdio.h>
#include <string.h>
#include "proxy_server_conf.h"
#include "proxy_conf_parser.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CProxyConfParser::CProxyConfParser(const char *conf_path) : \
	CBaseConfParser(conf_path)
{

}

CProxyConfParser::~CProxyConfParser()
{

}

int CProxyConfParser::init_conf(void *pconf)
{
	CProxyServerConf *pproxy_conf = (CProxyServerConf *)pconf;
#ifdef _DEBUG
	assert(pproxy_conf);
#endif //_DEBUG

	pproxy_conf->bmaster_flag = false;
	pproxy_conf->nnamespace_power = 32;
	pproxy_conf->nreplica_count = 3;
	pproxy_conf->nvnode_count = 1000;
	
	return CBaseConfParser::init_conf(pproxy_conf);
}

int CProxyConfParser::set_value(const char *pkey, const char *pvalue, void *pconf)
{
	CProxyServerConf *pproxy_conf = (CProxyServerConf *)pconf;
#ifdef _DEBUG
	assert(pproxy_conf);
#endif //_DEBUG

	if(strcmp(pkey, "master_flag") == 0)
	{
		pproxy_conf->bmaster_flag = get_bool_value(pvalue);
	}
	else if(strcmp(pkey, "vnode_count") == 0)
	{
		pproxy_conf->nvnode_count = get_int_value(pvalue);
	}
	else if(strcmp(pkey, "replica_count") == 0)
	{
		pproxy_conf->nreplica_count = get_int_value(pvalue);
	}
	else if(strcmp(pkey, "namespace_power") == 0)
	{
		pproxy_conf->nnamespace_power = get_int_value(pvalue);
	}
	else
	{
		return CBaseConfParser::set_value(pkey, pvalue, pconf);
	}
	return 0;
}