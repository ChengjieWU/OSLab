#include "types.h"
#include "printf.h"
#include "proc.h"
#include "semaphore.h"

void test_main()
{
	semaphore *sem = sem_open(0);
	
	fork();
	fork();
	fork();
	fork();
	
	int t = getpid();
	
	int i;
	
	for (i = 0; i < 10000; i++)
	{
		sem_wait(sem);
		if (i == 9999) printf("%d OK!\n", t);
		sem_post(sem);
	}
	exit(0);
}
