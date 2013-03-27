#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <strings.h>
#include <fcntl.h>
#include "file.h"

int main(int argc, char *argv[])
{
	CFile *fileptr = new CFile("test1.txt", O_RDWR | \
		O_CREAT | O_APPEND, S_IRWXU);
	if(fileptr == NULL){
		printf("create file: test1.txt failed\n");
		return -1;
	}
	
	srand(time(NULL));

	int i;
	unsigned char urand;
	unsigned char buf[128];
	for(i = 0; i < 127; i++){
		urand = rand() % 256;
		buf[i] = urand;
	}
	buf[127] = 0;
	int nwrite = fileptr->Write(buf, 128);
	printf("write %d character, context is:\n", nwrite);
	for(i = 0; i < 128; i++){
		printf("%d", buf[i]);
	}

	bzero(buf, 128);
	CFile *read_file_ptr = new CFile("test1.txt", O_RDONLY);
	int nread = read_file_ptr->Read(buf, 128);
	printf("read %d character from file," \
		" context is: %s\n", nread, buf);

	delete fileptr;
	delete read_file_ptr;

	return 0;
}