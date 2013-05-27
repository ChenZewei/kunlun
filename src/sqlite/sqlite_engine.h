#ifndef KL_SQLITE_ENGINE_H_
#define KL_SQLITE_ENGINE_H_
#include "sqlite3.h"
class CSqliteEngine
{
public:
	CSqliteEngine();
	/*
	 * @brief: if open sqlite db failed, throw the err msg string
	 */
	CSqliteEngine(const char *db_path);
	~CSqliteEngine();

	int sqlite_db_open(const char *db_path);
	/*
	 * @brief: execute the sql cmd
	 */
	int sqlite_tb_exec(const char *sql_str, \
		int (*callback)(void*,int,char**,char**), void *data);
private:
	sqlite3 *m_psqlite_db;
};
#endif //KL_SQLITE_ENGINE_H_