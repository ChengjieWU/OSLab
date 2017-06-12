#include "types.h"
#include "printf.h"
#include "proc.h"
#include "filesystem.h"
#include "video.h"
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

unsigned char buffer[50];
unsigned char gImage_FAILURE[1440000];

void test_main()
{
	/*int fd = fopen("haha\0", FS_READ);
	int len = fread(fd, buffer, 50);
	printf("Total length read is %d: ", len);
	fclose(fd);
	int i;
	for (i = 0; i < 50; i++)
		printf("%c", buffer[i]);*/
	
	int fd = fopen("hehe\0", FS_READ);
	int len = fread(fd, gImage_FAILURE, 1440000);
	printf("Total byte read is: %d\n", len);
	int i;
	for (i = 0; i < 10; i++)
	{
		printf("0x%x, ", gImage_FAILURE[i]);
	}
	fclose(fd);	
	fullVideo(gImage_FAILURE);
	exit();
}
