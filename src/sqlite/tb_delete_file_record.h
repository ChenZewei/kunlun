#ifndef KL_SQLITE_TB_DELETE_FILE_RECORD_H_
#define KL_SQLITE_TB_DELETE_FILE_RECORD_H_
class CSqliteEngine;
class CTbDeleteFileRecord
{
public:
	CTbDeleteFileRecord(const char *db_path);
	~CTbDeleteFileRecord();

	int create_table();
private:
	CSqliteEngine *m_psqlite_engine;
};
#endif //KL_SQLITE_TB_DELETE_FILE_RECORD_H_