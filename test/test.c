#include "types.h"
#include "printf.h"
#include "proc.h"

#include "filesystem.h"
#include "string.h"

#define t1
//#define t2
//#define t3


#ifdef t1

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
}
#endif

#ifdef t2
/* To test fcb among pcbs. */

unsigned char buffer[100];

void test_main()
{
	int fd = fopen("123\0", FS_WR);
	memset(buffer, 1, 100);
	fwrite(fd, buffer, 100);
	flseek(fd, 0, SEEK_SET);
	printf("%d", fread(fd, buffer, 30));
	flseek(fd, 0, SEEK_SET);
	fork();
	if (getpid() == 1) fclose(fd);
	else
	{
		int i = 0;
		for (i = 0; i < 0x0fffffff; i++);
		printf("%d", fread(fd, buffer, 30));
	}
	exit();
}

#endif

#ifdef t3
/* copy file test. one can use read_myfs to distract and use diff command to compre. */

unsigned char buf[512];

void test_main()
{
	int fd = fopen("kernel.bin\0", FS_READ);
	int fd2 = fopen("wcj\0", FS_WRITE);
	int len = flseek(fd, 0, SEEK_END);
	flseek(fd, 0, SEEK_SET);
	int block = len / 512;
	int rest = len % 512;
	int i;
	for (i = 0; i < block; i++)
	{
		fread(fd, buf, 512);
		fwrite(fd2, buf, 512);
	}
	fread(fd, buf, rest);
	fwrite(fd2, buf, rest);
	fclose(fd);
	fclose(fd2);
	exit();
}
#endif
