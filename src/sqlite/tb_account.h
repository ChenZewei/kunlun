#ifndef KL_SQLITE_TB_ACCOUNT_H_
#define KL_SQLITE_TB_ACCOUNT_H_
class CSqliteEngine;
class CTbAccount
{
public:
	CTbAccount(const char *db_path);
	~CTbAccount();

	int create_table();
private:
	CSqliteEngine *m_psqlite_engine;
};
#endif //KL_SQLITE_TB_ACCOUNT_H_