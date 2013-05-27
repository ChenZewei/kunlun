#include <stdio.h>
#include "log.h"
#include "sqlite3.h"
#include "sqlite_engine.h"

CSqliteEngine::CSqliteEngine() : m_psqlite_db(NULL)
{

}

CSqliteEngine::CSqliteEngine(const char *db_path)
{
	if(sqlite_db_open(db_path) != SQLITE_OK)
	{
		throw sqlite3_errmsg(m_psqlite_db);
	}
}

CSqliteEngine::~CSqliteEngine()
{
	if(m_psqlite_db != NULL)
	{
		sqlite3_close(m_psqlite_db);
		m_psqlite_db = NULL;
	}
}

int CSqliteEngine::sqlite_db_open(const char *db_path)
{
	int ret;
	if((ret = sqlite3_open(db_path, &m_psqlite_db)) != SQLITE_OK)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"sqlite engine can't open database file(db_path: %s), err: %s", \
			__LINE__, db_path, sqlite3_errmsg(m_psqlite_db));
		return (ret = ret == 0 ? SQLITE_ERROR : ret);
	}
	return 0;
}

int CSqliteEngine::sqlite_tb_exec(const char *sql_str, \
	int (*callback)(void*,int,char**,char**), void *data)
{
	int ret;
	char *perrmsg;
	if((ret = sqlite3_exec(m_psqlite_db, sql_str, callback, data, &perrmsg)) != SQLITE_OK)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"sqlite engine execute sql(%s) failed, err: %s", \
			__LINE__, sql_str, perrmsg);
		return (ret = ret == 0 ? SQLITE_ERROR : ret);
	}
	return 0;
}