#ifndef KL_COMMON_BASE_CONF_PARSER_H_
#define KL_COMMON_BASE_CONF_PARSER_H_
#include <stdint.h>
class CBaseServerConf;
class CBaseConfParser
{
public:
	/*
	 * @brief: if call CBaseConfParser constructor failed, throw an errcode
	 */
	CBaseConfParser(const char *conf_path);
	~CBaseConfParser();

	int parse_conf(void *pconf);
protected:
	/*
	 * @brief: load the data of conf file to buffer
	 * @param: conf_path, the path of conf file
	 @ @return: if successed, return 0, otherwise, return an errcode
	 */
	int load_conf_file(const char *conf_path);
	/*
	 * @brief: parse every line of conf file, change to key-value format
	 * @param: pline, a line of conf file data
	 * @param: pconf, pconf buf to save conf data
	 */
	int parse_line(char **pline, void *pconf, bool &blast_line);
	/*
	 * @brief: if string follow by character ' ', change to '\0'
	 * @return: if successed, return the pointer of string start, otherwise, return NULL
	 */
	const char *format_string(char *pstart, char *pend);
	virtual int set_value(const char *pkey, const char *pvalue, void *pconf);
	virtual int init_conf(void *pconf);
	int get_int_value(const char *str);
	int get_log_level(const char *str);
	int get_bit_size(const char *str);
	bool get_bool_value(const char *str);
	const char *str_error(int errcode);

	char *m_pconf_buf;
};
#endif //KL_COMMON_BASE_CONF_PARSER_H_