#include "types.h"
#include "printf.h"
#include "proc.h"

#include "filesystem.h"
#include "string.h"

/*
void test_main()
{
	fork();
	if (getpid() == 0) fork();
	if (getpid() == 0) fork();
	if (getpid() == 1) fork();
	printf("I'm process No.%d\n", getpid());
	while (1)
	{
		int t = getpid();
		if (t == 3) 
		{
			sleep(4000);
			printf("*********\n");
		}
		else if (t == 2) 
		{
			sleep(2000);
			printf("******\n");
		}
		else if (t == 1) 
		{
			sleep(1000);
			printf("***\n");
		}
		else if (t == 4) 
		{
			sleep(8000);
			printf("************\n");
		}
	}
	exit();
}*/

#define blocksize 512

unsigned char buffer[blocksize];

void test_main()
{
	printf("read and write test\n");
	
	int fd = fopen("kernel.bin", FS_READ);
	int fd2 = fopen("newfile.bin", FS_WRITE);
	
	unsigned len = flseek(fd, 0, SEEK_END);
	int blocknum = len / blocksize;
	int restbyte = len % blocksize;
	
	printf("%u\n", len);
	flseek(fd, 0, SEEK_SET);
	
	int i;
	for (i = 0; i < blocknum; i++)
	{
		fread(fd, buffer, blocksize);
		//printf("%d\n", i);
		fwrite(fd2, buffer, blocksize);
	}
	fread(fd, buffer, restbyte);
	fwrite(fd2, buffer, restbyte);
	
	fclose(fd);
	fclose(fd2);
	
	exit();
}
