#include <new>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "sqlite_engine.h"
#include "tb_delete_file_record.h"

CTbDeleteFileRecord::CTbDeleteFileRecord(const char *db_path)
{
	try
	{
		m_psqlite_engine = new CSqliteEngine(db_path);
	}
	catch(std::bad_alloc)
	{
		m_psqlite_engine = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create sqilte engine", \
			__LINE__);
		throw ENOMEM;
	}
	catch(const char *perrmsg)
	{
		m_psqlite_engine = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"tb_hash_code create sqlite engine failed, err: %s", \
			__LINE__, perrmsg);
		throw perrmsg;
	}

	if(create_table() != 0)
	{
		throw "create table tb_delete_file_record failed";
	}
}

CTbDeleteFileRecord::~CTbDeleteFileRecord()
{
	if(m_psqlite_engine != NULL)
	{
		delete m_psqlite_engine;
		m_psqlite_engine = NULL;
	}
}

int CTbDeleteFileRecord::create_table()
{
	return 0;
}