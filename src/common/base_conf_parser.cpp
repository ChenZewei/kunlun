#include <new>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "base_server_conf.h"
#include "base_conf_parser.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

#define KL_COMMON_CONF_KEY_ERR	1
#define KL_COMMON_CONF_VALUE_ERR	2

CBaseConfParser::CBaseConfParser(const char *conf_path) : \
	m_pconf_buf(NULL)
{
	int ret;

	if((ret = load_conf_file(conf_path)) != 0)
	{
		throw ret;
	}
}

CBaseConfParser::~CBaseConfParser()
{
	if(m_pconf_buf != NULL)
	{
		delete [] m_pconf_buf;
		m_pconf_buf = NULL;
	}
}

int CBaseConfParser::load_conf_file(const char *conf_path)
{
	int64_t nfile_size;
	try
	{
		CFile conf_file(conf_path, O_RDONLY);
		if((nfile_size = conf_file.get_file_size()) <= 0)
		{
			errno = (errno == 0 ? EEXIST : errno);
			fprintf(stderr, "file: "__FILE__", line: %d, " \
				"get conf file size failed, conf_path: %s, err: %s\n", 
				__LINE__, conf_path, strerror(errno));
			return errno;
		}

		m_pconf_buf = new char[nfile_size + 1];
		if(conf_file.read_file(m_pconf_buf, nfile_size) != nfile_size)
		{
			errno = (errno == 0 ? EACCES : errno);
			fprintf(stderr, "file: "__FILE__", line: %d, " \
				"read conf file failed, conf_path: %s, err: %s\n", 
				__LINE__, conf_path, strerror(errno));
			return errno;
		}
	}
	catch(std::bad_alloc)
	{
		fprintf(stderr, "file: "__FILE__", line: %d, " \
			"no more memory to load conf file, conf_path: %s\n", \
			__LINE__, conf_path);
		return ENOMEM;
	}
	catch(int errcode)
	{
		fprintf(stderr, "file: "__FILE__", line: %d, " \
			"load conf file failed, conf_path: %s, err: %s\n", \
			__LINE__, conf_path, strerror(errcode));
		return errcode;
	}
	return 0;
}

int CBaseConfParser::parse_conf(void *pconf)
{
	char *p;
	int ret;
	bool blast_line;

	init_conf(pconf);
	
	p = m_pconf_buf;
	//printf("p = %s\n", p);
	while(*p != 0)
	{
		if((ret = parse_line(&p, pconf, blast_line)) != 0)
		{
			fprintf(stderr, "file: "__FILE__", line: %d, " \
				"parse conf line to key-value failed, err: %s\n", \
				__LINE__, str_error(ret));
			return ret;
		}
		if(blast_line)
			break;
		p++; //to the next line
	}
	return ret;
}

int CBaseConfParser::init_conf(void *pconf)
{
	CBaseServerConf *pbase_conf;

	pbase_conf = (CBaseServerConf *)(pconf);
#ifdef _DEBUG
	assert(pbase_conf != NULL);
#endif //_DEBUG
	//set default conf value
	memset(pbase_conf->bind_host, 0, 256);
	pbase_conf->nbind_port = 6000;
	pbase_conf->nlog_level = 4; //debug log level
	pbase_conf->nthread_stack_size = 1 * 1024 * 1024; //1MB
	pbase_conf->ntimeout = 5; //5 seconds
	pbase_conf->nwork_thread_count = 100;
	memset(pbase_conf->sys_log_path, 0, 256);

	return 0;
}

int CBaseConfParser::parse_line(char **pline, void *pconf, bool &blast_line)
{
	char *pstart, *pend;
	const char *pkey, *pvalue;
	
	pstart = *pline;
	pend = *pline;
	//ignore annotation
	if(*pstart == '#')
	{
		while(*pend != '\n' && *pend != 0)
			pend++;

		*pline = pend;
		blast_line = (*pend == 0 ? true : false);
		return 0;
	}
	//ignore null string
	while(*pend == ' ')
	{
		pend++;
	}
	if((*pend == '\r' && *(++pend) == '\n') || \
		*pend == '\n' || *pend == 0)
	{
		*pline = pend;
		blast_line = (*pend == 0 ? true : false);
		return 0;
	}
	//get key string
	while(*pend != '=' && *pend != '\n' && *pend != 0)
	{
		pend++;
	}
	if(*pend != '=' || pstart == pend)
	{
		return KL_COMMON_CONF_KEY_ERR;
	}
	if((pkey = format_string(pstart, pend - 1)) == NULL)
	{
		fprintf(stderr, "file: "__FILE__", line: %d, " \
			"conf file key is illegal\n", \
			__LINE__);
		return KL_COMMON_CONF_KEY_ERR;
	}

	//get value string
	pstart = ++pend;
	while(*pend != '\r' && *pend != '\n' && *pend != 0)
	{
		pend++;
	}
	if(pstart == pend)
	{
		return KL_COMMON_CONF_VALUE_ERR;
	}
	if((pvalue = format_string(pstart, pend - 1)) == NULL)
	{
		fprintf(stderr, "file: "__FILE__", line: %d, " \
			"conf file value is illegal\n", \
			__LINE__);
		return KL_COMMON_CONF_VALUE_ERR;
	}

	blast_line = (*pend == 0 ? true : false);
	if(*pend == '\r')
	{
		*pend = 0;
		*(++pend) = 0;
	}
	else
	{
		*pend = 0;
	}
	*pline = pend;
	return set_value(pkey, pvalue, pconf);
}

int CBaseConfParser::set_value(const char *pkey, const char *pvalue, void *pconf)
{
	CBaseServerConf *pbase_conf = (CBaseServerConf *)pconf;
#ifdef _DEBUG
	assert(pbase_conf != NULL);
#endif //_DEBUG
	//printf("key = %s, value = %s\n", pkey, pvalue);

	if(strcmp(pkey, "bind_host") == 0)
	{
		strcpy(pbase_conf->bind_host, pvalue);
	}
	else if(strcmp(pkey, "bind_port") == 0)
	{
		pbase_conf->nbind_port = get_int_value(pvalue);
	}
	else if(strcmp(pkey, "work_thread_count") == 0)
	{
		pbase_conf->nwork_thread_count = get_int_value(pvalue);
	}
	else if(strcmp(pkey, "timeout") == 0)
	{
		pbase_conf->ntimeout = get_int_value(pvalue);
	}
	else if(strcmp(pkey, "sys_log_path") == 0)
	{
		strcpy(pbase_conf->sys_log_path, pvalue);
	}
	else if(strcmp(pkey, "log_level") == 0)
	{
		pbase_conf->nlog_level = get_log_level(pvalue);
	}
	else if(strcmp(pkey, "thread_stack_size") == 0)
	{
		pbase_conf->nthread_stack_size = get_bit_size(pvalue);
	}
	else
	{
		return -1;
	}
	return 0;
}

const char *CBaseConfParser::format_string(char *pstart, char *pend)
{
	while(*pstart == ' ' && pstart != pend)
	{
		*pstart = 0;
		pstart++;
	}
	if(*pstart == ' ')
		return NULL;

	while(*pend == ' ' && pend != pstart)
	{
		*pend = 0;
		pend--;
	}

	return pstart;
}

int CBaseConfParser::get_int_value(const char *str)
{
	//printf("int str = %s\n", str);
	//printf("%c\n", str + strlen(str) - 1);
	return atoi(str);
}

bool CBaseConfParser::get_bool_value(const char *str)
{
	if(strcmp(str, "true") == 0)
	{
		return true;
	}
	return false;
}

const char *CBaseConfParser::str_error(int errcode)
{
	switch(errcode)
	{
	case 0:
		return "parse conf file successed";
	case KL_COMMON_CONF_KEY_ERR:
		return "parse conf key error";
	case KL_COMMON_CONF_VALUE_ERR:
		return "parse conf value error";
	default:
		return "undefined conf parseing error";
	}
	return NULL;
}

int CBaseConfParser::get_log_level(const char *str)
{
	if(strcmp(str, "error") == 0)
	{
		return 0;
	}
	else if(strcmp(str, "warnning") == 0)
	{
		return 1;
	}
	else if(strcmp(str, "notice") == 0)
	{
		return 2;
	}
	else if(strcmp(str, "info") == 0)
	{
		return 3;
	}
	else if(strcmp(str, "debug") == 0)
	{
		return 4;
	}

	return -1;
}

int CBaseConfParser::get_bit_size(const char *str)
{
	int n;
	const char *p;
	char num[11];

	p = str;
	while(*p != 0)
	{
		if(*p > 9 + '0' || *p < '0')
			break;
		p++;
	}
	memset(num, 0, sizeof(num));
	memcpy(num, str, p - str);
	//printf("num = %s\n", num);
	n = atoi(num);
	//printf("n = %d\n", n);
	//printf("p = %s\n", p);
	if(strcmp(p, "") == 0)
	{
		return n;
	}
	else if(strcmp(p, "KB") == 0)
	{
		return n * 1024;
	}
	else if(strcmp(p, "MB") == 0)
	{
		return n * 1024 * 1024;
	}
	else if(strcmp(p, "GB") == 0)
	{
		return n * 1024 * 1024 * 1024;
	}
	else
	{
		return 0;
	}
}