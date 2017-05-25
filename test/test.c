#include "types.h"
#include "printf.h"
#include "proc.h"

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


int share;

void test_main()
{
	share = 0;
	printf("%d\n", share);
	int t = fork();
	if (t == 1) {
		printf("%d\n", share);
		sleep(100);
		printf("%d\n", share);
	}
	else {
		share = 1;
		while(1);
	}
	exit();
}
