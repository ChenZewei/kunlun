#include <new>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "hash.h"

int get_rand_str(char *buf, int nsize)
{
	int i, n;

	memset(buf, 0, nsize);
	for(i = 0; i < nsize - 1; i++)
	{
		if((n = rand()) % 2 == 0)
		{
			//get char
			n = rand() % 26;
			buf[i] = n + 'a';
		}
		else
		{
			//get number
			n = rand() % 10;
			buf[i] = n + '0';
		}
	}
	return 0;
}

int main(int argc, char *argv[]) 
{

	//cout << MD5("abc").toString() << endl;
	//cout << MD5(ifstream("D:\\test.txt")).toString() << endl;
	//cout << MD5(ifstream("D:\\test.exe", ios::binary)).toString() << endl;
	//cout << FileDigest("D:\\test.exe") << endl;
	char md5_code[33];

	MD5 md5;
	
	md5.update("", strlen(""));
	printf("md5:\"\" = %s\n", md5.to_string(md5_code, 33));
	md5.update("a", strlen("a"));
	printf("md5:\"a\" = %s\n", md5.to_string(md5_code, 33));
	md5.update("bc", strlen("bc"));
	printf("md5:\"abc\" = %s\n", md5.to_string(md5_code, 33));
	md5.update("defghijklmnopqrstuvwxyz", strlen("defghijklmnopqrstuvwxyz"));
	printf("md5:\"abcdefghijklmnopqrstuvwxyz\" = %s\n", md5.to_string(md5_code, 33));

	md5.reset();
	md5.update("message digest", strlen("message digest"));
	printf("md5:\"message digest\" = %s\n", md5.to_string(md5_code, 33));

	md5.reset();
	CFile file(argv[1], O_RDONLY);
	md5.update(&file);
	printf("md5:\"file: %s\" = %s\n", argv[1], md5.to_string(md5_code, 33));
	//PrintMD5("D:\\test.txt", md5);

	int i;
	CNameSpaceHash namespace_hash(32, 1000);
	char account_name[5];
	char container_name[5];
	char object_name[5];
	char file_path[20];
	/*printf("account/container/object -> vnode_id: %d\n", \
		namespace_hash.get_vnode_id("account/container/object"));
	printf("account1/container1/object1 -> vnode_id: %d\n", \
		namespace_hash.get_vnode_id("account1/container1/object1"));*/
	srand(time(NULL));
	for(i = 0; i < 100; i++)
	{
		memset(file_path, 0, sizeof(file_path));
		get_rand_str(account_name, sizeof(account_name));
		get_rand_str(container_name, sizeof(container_name));
		get_rand_str(object_name, sizeof(object_name));
		snprintf(file_path, sizeof(file_path), "%s/%s/%s", \
			account_name, container_name, object_name);
		printf("file_path: %s -> vnode_id: %d\n", \
			file_path, namespace_hash.get_vnode_id(file_path));
	}
	return 0;
}
