#include "types.h"
#include "printf.h"
#include "proc.h"

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
		if (t == 1) 
		{
			printf("*********\n");
			sleep(4000);
		}
		else if (t == 2) 
		{
			printf("******\n");
			sleep(2000);
		}
		else if (t == 3) 
		{
			printf("***\n");
			sleep(1000);
		}
		else if (t == 4) 
		{
			printf("************\n");
			sleep(8000);
		}
	}
	exit();
}
