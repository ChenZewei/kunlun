#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "sqlite3.h"

int select_callback(void * data, int col_count, char ** col_values, char ** col_Name);
typedef struct _user
{
	char user_id[20];
	int age;
	char birthday[50];
}user;

int main(int argc, char * argv[])
{
	const char * sSQL1 = "create table users(userid varchar(20) PRIMARY KEY, age int, birthday datetime);";
	char * pErrMsg = 0;
	int result = 0;
	sqlite3 * db = NULL;
	int ret = sqlite3_open("./test.db", &db);
	if( ret != SQLITE_OK ) {
		fprintf(stderr, "open database failed: %s", sqlite3_errmsg(db));
		return(1);
	}
	printf("database connect successed!\n");

	sqlite3_exec( db, sSQL1, 0, 0, &pErrMsg );
	if( ret != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", pErrMsg);
		sqlite3_free(pErrMsg);
	}

	result = sqlite3_exec( db, "insert into users values('weiyuchen',20,'2011-7-23');", 0, 0, &pErrMsg);
	if(result == SQLITE_OK){
		printf("insert successed\n");
	}
	result = sqlite3_exec( db, "insert into users values('chengen',20,'2012-9-20');", 0, 0, &pErrMsg);
	if(result == SQLITE_OK){
		printf("insert successed\n");
	}

	printf("query data successed\n");
	std::vector<user> user_list;
	int i;
	user user_record;
	sqlite3_exec( db, "select * from users;", select_callback, (void*)&user_list, &pErrMsg);
	for (i = 0; i < user_list.size(); i++)
	{
		user_record = user_list[i];
		printf("%c, %s, %d, %s\n", user_record.user_id[0], user_record.user_id, user_record.age, user_record.birthday);
	}
	

	sqlite3_close(db);
	db = 0;
	printf("close database!\n");

	return 0;
}

int select_callback(void * data, int col_count, char ** col_values, char ** col_Name)
{
	/*int i;
	for( i=0; i < col_count; i++){
		printf( "%s = %s\n", col_Name[i], col_values[i] == 0 ? "NULL" : col_values[i] );
	}*/
	if(col_count != 3)
	{
		printf("error\n");
		return -1;
	}
	user user_record;
	std::vector<user> *puser_list;
	puser_list = (std::vector<user> *)data;
	memset(&user_record, 0, sizeof(user_record));
	strcpy(user_record.user_id, col_values[0]);
	user_record.age = atoi(col_values[1]);
	strcpy(user_record.birthday, col_values[2]);
	puser_list->push_back(user_record);

	return 0;
}
