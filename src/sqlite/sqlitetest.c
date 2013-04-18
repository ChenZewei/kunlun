#include <stdio.h>
#include "sqlite3.h"

//��ѯ�Ļص���������
int select_callback(void * data, int col_count, char ** col_values, char ** col_Name);

int main(int argc, char * argv[])
{
	const char * sSQL1 = "create table users(userid varchar(20) PRIMARY KEY, age int, birthday datetime);";
	char * pErrMsg = 0;
	int result = 0;
	// �������ݿ�
	sqlite3 * db = NULL;
	int ret = sqlite3_open("./test.db", &db);
	if( ret != SQLITE_OK ) {
		fprintf(stderr, "�޷������ݿ�: %s", sqlite3_errmsg(db));
		return(1);
	}
	printf("���ݿ����ӳɹ�!\n");

	// ִ�н���SQL
	sqlite3_exec( db, sSQL1, 0, 0, &pErrMsg );
	if( ret != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", pErrMsg);
		sqlite3_free(pErrMsg);
	}

	// ִ�в����¼SQL
	result = sqlite3_exec( db, "insert into users values('����',20,'2011-7-23');", 0, 0, &pErrMsg);
	if(result == SQLITE_OK){
		printf("�������ݳɹ�\n");
	}
	result = sqlite3_exec( db, "insert into users values('����',20,'2012-9-20');", 0, 0, &pErrMsg);
	if(result == SQLITE_OK){
		printf("�������ݳɹ�\n");
	}

	// ��ѯ���ݱ�
	printf("��ѯ���ݿ�����\n");
	sqlite3_exec( db, "select * from users;", select_callback, 0, &pErrMsg);

	// �ر����ݿ�
	sqlite3_close(db);
	db = 0;
	printf("���ݿ�رճɹ�!\n");

	return 0;
}

int select_callback(void * data, int col_count, char ** col_values, char ** col_Name)
{
	// ÿ����¼�ص�һ�θú���,�ж������ͻص����ٴ�
	int i;
	for( i=0; i < col_count; i++){
		printf( "%s = %s\n", col_Name[i], col_values[i] == 0 ? "NULL" : col_values[i] );
	}

	return 0;
}
