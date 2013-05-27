#include <new>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "tb_hashcode.h"
#include "sqlite_engine.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CTbHashCode::CTbHashCode(const char *db_path)
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

	if(create_tb_hashcode() != 0)
	{
		throw "create table tb_hashcode failed";
	}
}

CTbHashCode::~CTbHashCode()
{
	if(m_psqlite_engine != NULL)
	{
		delete m_psqlite_engine;
		m_psqlite_engine = NULL;
	}
}

int CTbHashCode::create_tb_hashcode()
{
	int ret;
	char sql_str[256];
	
	memset(sql_str, 0, sizeof(sql_str));
	snprintf(sql_str, sizeof(sql_str), "create table if not exists " \
		"tb_hashcode(file_path varchar(256) PRIMARY KEY, hash_code varchar(33));");
	if((ret = m_psqlite_engine->sqlite_tb_exec(sql_str, NULL, NULL)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create table tb_hashcode failed, errcode: %d", \
			__LINE__, ret);
		return ret;
	}
	return 0;
}

int CTbHashCode::insert_record(CVnodeHashCode *pvnode_hashcode)
{
	int ret;
	char sql_str[256];
#ifdef _DEBUG
	assert(pvnode_hashcode != NULL);
#endif //_DEBUG
	memset(sql_str, 0, sizeof(sql_str));
	snprintf(sql_str, sizeof(sql_str), "insert into tb_hashcode values('%s', '%s');", \
		pvnode_hashcode->file_path, pvnode_hashcode->hash_code);
	if((ret = m_psqlite_engine->sqlite_tb_exec(sql_str, NULL, NULL)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"insert values('%s', '%s') to table tb_hashcode failed, errcode: %d", \
			__LINE__, pvnode_hashcode->file_path, pvnode_hashcode->hash_code, ret);
		return ret;
	}
	return 0;
}

int CTbHashCode::update_hash_code(CVnodeHashCode *pvnode_hashcode)
{
	int ret;
	char sql_str[256];
#ifdef _DEBUG
	assert(pvnode_hashcode != NULL);
#endif //_DEBUG
	memset(sql_str, 0, sizeof(sql_str));
	snprintf(sql_str, sizeof(sql_str), "update tb_hashcode set hash_code = '%s' " \
		"where file_path = '%s';", \
		pvnode_hashcode->hash_code, pvnode_hashcode->file_path);
	if((ret = m_psqlite_engine->sqlite_tb_exec(sql_str, NULL, NULL)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"update hash_code: %s, file_path: %s failed, errcode: %d", \
			__LINE__, pvnode_hashcode->hash_code, pvnode_hashcode->file_path, ret);
		return ret;
	}
	return 0;
}

int CTbHashCode::query_all_hashcode(hashcode_list_t *phashcode_list)
{
	int ret;
	char sql_str[256];
#ifdef _DEBUG
	assert(phashcode_list != NULL);
#endif //_DEBUG
	memset(sql_str, 0, sizeof(sql_str));
	snprintf(sql_str, sizeof(sql_str), "select * from tb_hashcode;");
	if((ret = m_psqlite_engine->sqlite_tb_exec(sql_str, select_callback, \
		(void*)phashcode_list)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"query all file hash code failed, errcode: %d", \
			__LINE__, ret);
		return ret;
	}
	return 0;
}

int CTbHashCode::query_hashcode_by_filepath(char *phashcode, int size, const char *pfile_path)
{
	int ret;
	char sql_str[256];
	hashcode_list_t hc_list;
	CVnodeHashCode vnode_hashcode;
#ifdef _DEBUG
	assert(phashcode != NULL && size > 0);
	assert(pfile_path != NULL);
#endif //_DEBUG
	memset(phashcode, 0, size);
	snprintf(sql_str, sizeof(sql_str), "select * from tb_hashcode where file_path = '%s';", \
		pfile_path);
	if((ret = m_psqlite_engine->sqlite_tb_exec(sql_str, select_callback, (void*)&hc_list)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"query hash code by file_path: %s failed, errcode: %d", \
			__LINE__, pfile_path, ret);
		return ret;
	}
	if(hc_list.size() == 0)
		return -2; //record not exist

	if(hc_list.size() != 1)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the hash code of file_path: %s is not single", \
			__LINE__, pfile_path);
		return -1;
	}
	vnode_hashcode = hc_list.front();
	if(size < strlen(vnode_hashcode.hash_code) + 1)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the hasecode buffer size less than the min needed length", \
			__LINE__);
		return -1;
	}
	strcpy(phashcode, vnode_hashcode.hash_code);
	return 0;
}

int CTbHashCode::delete_record(char *pfile_path)
{
	int ret;
	char sql_str[256];

#ifdef _DEBUG
	assert(pfile_path != NULL);
#endif //_DEBUG
	memset(sql_str, 0, sizeof(sql_str));
	snprintf(sql_str, sizeof(sql_str), "delete from tb_hashcode where file_path = '%s';", \
		pfile_path);
	if((ret = m_psqlite_engine->sqlite_tb_exec(sql_str, NULL, NULL)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"delete record(file_path: %s) from table tb_hashcode failed, errcode: %d", \
			__LINE__, pfile_path, ret);
		return ret;
	}
	return 0;
}

int CTbHashCode::select_callback(void *data, int col_count, char **col_values, char **col_name)
{
	int i;
	CVnodeHashCode vnode_hashcode;
	hashcode_list_t *phashcode_list;
	phashcode_list = (hashcode_list_t*)data;
#ifdef _DEBUG
	assert(phashcode_list != NULL);
	assert(col_count == 2);
#endif //_DEBUG
	memset(&vnode_hashcode, 0, sizeof(vnode_hashcode));
	for(i = 0; i < col_count; i++)
	{
		if(strcmp(col_name[i], "file_path") == 0)
		{
			strcpy(vnode_hashcode.file_path, col_values[i]);
		}
		else if(strcmp(col_name[i], "hash_code") == 0)
		{
			strcpy(vnode_hashcode.hash_code, col_values[i]);
		}
		else
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"catch an undefined col_name: %s", \
				__LINE__, col_name[i]);
			return -1;
		}
	}
	phashcode_list->push_back(vnode_hashcode);
	return 0;
}