#include "types.h"
#include "printf.h"
#include "proc.h"
#include "wthread.h"

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

void subfunction1(int t)
{
	int a = t;
	int id = getpid();
	printf("Hey, this is in the subfunction: %x, and I'm process NO.%d\n", a, id);
	wthread_exit();
}

void subfunction2(int t)
{
	int a = t;
	int id = getpid();
	printf("Hallo, this is in the subfunction: %x, and I'm process NO.%d\n", a, id);
	wthread_exit();
}

void subfunction3(int t)
{
	int a = t;
	int id = getpid();
	printf("Halo, this is in the subfunction: %x, and I'm process NO.%d\n", a, id);
	wthread_exit();
}

void test_main()
{
	printf("Hallo, I'm process NO.%d\n", getpid());
	wthread_create(&subfunction1, (void *)66);
	wthread_create(&subfunction2, (void *)666);
	wthread_create(&subfunction3, (void *)6666);
	while(1);
}
