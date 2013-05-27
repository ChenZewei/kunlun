#ifndef KL_SQLITE_TB_CONTAINER_H_
#define KL_SQLITE_TB_CONTAINER_H_
class CSqliteEngine;
class CTbContainer
{
public:
	CTbContainer(const char *db_path);
	~CTbContainer();

	int create_table();
private:
	CSqliteEngine *m_psqlite_engine;
};
#endif //KL_SQLITE_TB_CONTAINER_H_