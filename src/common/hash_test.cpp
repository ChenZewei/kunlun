#include <new>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "file.h"
#include "hash.h"

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
	return 0;
}
