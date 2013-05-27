#ifndef KL_PROXY_CONF_PARSER_H_
#define KL_PROXY_CONF_PARSER_H_
#include "base_conf_parser.h"
class CProxyConfParser : public CBaseConfParser
{
public:
	CProxyConfParser(const char *conf_path);
	~CProxyConfParser();

protected:
	virtual int set_value(const char *pkey, const char *pvalue, void *pconf);
	virtual int init_conf(void *pconf);
};
#endif //KL_PROXY_CONF_PARSER_H_