#ifndef KL_STORAGE_CONF_PARSER_H_
#define KL_STORAGE_CONF_PARSER_H_
#include "base_conf_parser.h"
class CInetAddr;
class CStorageConfParser : public CBaseConfParser
{
public:
	CStorageConfParser(const char *conf_path);
	~CStorageConfParser();

protected:
	virtual int set_value(const char *pkey, const char *pvalue, void *pconf);
	virtual int init_conf(void *pconf);
	int get_addr_value(const char *pvalue, CInetAddr *paddr, bool &bmaster_flag);
};
#endif //KL_STORAGE_CONF_PARSER_H_