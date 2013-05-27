#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage_server_conf.h"
#include "storage_conf_parser.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CStorageConfParser::CStorageConfParser(const char *conf_path) : \
	CBaseConfParser(conf_path)
{

}

CStorageConfParser::~CStorageConfParser()
{

}

int CStorageConfParser::init_conf(void *pconf)
{
	CStorageServerConf *pstorage_conf = (CStorageServerConf *)pconf;
#ifdef _DEBUG
	assert(pstorage_conf != NULL);
#endif //_DEBUG

	memset(pstorage_conf->device_path, 0, KL_COMMON_PATH_LEN);
	pstorage_conf->nweight = 1;
	pstorage_conf->nzone_id = 0;
	return CBaseConfParser::init_conf(pconf);
}

int CStorageConfParser::set_value(const char *pkey, const char *pvalue, void *pconf)
{
	CStorageServerConf *pstorage_conf = (CStorageServerConf *)pconf;
#ifdef _DEBUG
	assert(pstorage_conf != NULL);
#endif //_DEBUG
	//printf("pkey = %s, pvalue = %s\n", pkey, pvalue);
	if(strcmp(pkey, "zonde_id") == 0)
	{
		pstorage_conf->nzone_id = get_int_value(pvalue);
	}
	else if(strcmp(pkey, "weight") == 0)
	{
		pstorage_conf->nweight = get_int_value(pvalue);
	}
	else if(strcmp(pkey, "device_root") == 0)
	{
		//printf("pvalue  = %s\n", pvalue);
		strcpy(pstorage_conf->device_path, pvalue);
	}
	else if(strcmp(pkey, "proxy_server_addr") == 0)
	{
		bool bmaster_flag;
		CInetAddr proxy_addr;
		get_addr_value(pvalue, &proxy_addr, bmaster_flag);
		if(bmaster_flag == true)
		{
			pstorage_conf->proxy_addr_list.insert(pstorage_conf->proxy_addr_list.begin(), \
				proxy_addr);
		}
		else
		{
			pstorage_conf->proxy_addr_list.push_back(proxy_addr);
		}
	}
	else
	{
		return CBaseConfParser::set_value(pkey, pvalue, pconf);
	}
	return 0;
}

int CStorageConfParser::get_addr_value(const char *pvalue, CInetAddr *paddr, bool &bmaster_flag)
{
	char *ip_str;
	char *port_str;
	char *master_str;

	ip_str = const_cast<char*>(pvalue);
	port_str = ip_str;
	while(*port_str != ':' && *port_str != 0)
	{
		port_str++;
	}
	if(*port_str == ':')
	{
		*(port_str++) = 0;
	}
	master_str = port_str;
	while(*master_str != ',' && *master_str != 0)
	{
		master_str++;
	}
	if(*master_str == ',')
	{
		*(master_str++) = 0;
	}
	//printf("ip_str = %s, port_str = %s, master_str = %s\n", ip_str, port_str, master_str);
	paddr->setsockaddr(format_string(ip_str, port_str - 2), \
		atoi(format_string(port_str, master_str - 2)));

	if(strcmp(format_string(master_str, master_str + strlen(master_str) - 1), \
		"master") == 0)
	{
		bmaster_flag = true;
	}
	else
	{
		bmaster_flag = false;
	}
	return 0;
}