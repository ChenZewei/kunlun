#ifndef KL_SQLITE_TB_HASHCODE_H_
#define KL_SQLITE_TB_HASHCODE_H_
#include <list>
#include "common_types.h"
class CSqliteEngine;
class CVnodeHashCode
{
public:
	char file_path[KL_COMMON_PATH_LEN];
	char hash_code[KL_COMMON_MD5_HASH_LEN];
};
typedef std::list<CVnodeHashCode> hashcode_list_t;
typedef hashcode_list_t::iterator hashcode_iter_t;
class CTbHashCode
{
public:
	/*
	 * @brief: if create sqlite engine failed, throw a errmsg string
	           if no more memory to create sqlite engine, throw a errcode
	 */
	CTbHashCode(const char *db_path);
	~CTbHashCode();

	int create_tb_hashcode();
	int insert_record(CVnodeHashCode *pvnode_hashcode);
	int update_hash_code(CVnodeHashCode *pvnode_hashcode);
	int query_all_hashcode(hashcode_list_t *phashcode_list);
	int query_hashcode_by_filepath(char *phashcode, int size, \
		const char *pfile_path);
	int delete_record(char *pfile_path);
private:
	static int select_callback(void *data, int col_count, \
		char **col_values, char **col_name);

	CSqliteEngine *m_psqlite_engine;
};
#endif //KL_SQLITE_TB_HASHCODE_H_