#include "types.h"
#include "printf.h"
#include "proc.h"
#include "wthread.h"
#include "semaphore.h"

//#define named
#define unnamed

#ifdef named
void test_main()
{
	semaphore *sem = sem_open(0);
	
	fork();
	fork();
	fork();
	fork();
	
	int t = getpid();
	
	int i;
	
	for (i = 0; i < 100000; i++)
	{
		sem_wait(sem);
		if (i == 99999) printf("%d OK!\n", t);
		sem_post(sem);
	}
	exit();
}
#endif


#ifdef unnamed
semaphore sem;

void create(void* arg)
{
	int t = getpid();
	int i;
	for (i = 0; i < 10000; i++) {
		//printf("%d locks\n", t);
		sem_wait(&sem);
		if (i == 9999) printf("%d OK!\n", t);
		sem_post(&sem);
		//printf("%d unlocks\n", t);
	}
	wthread_exit();
}

void test_main()
{
	sem_init(&sem, 1);
	wthread_create(&create, (void *)1);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	wthread_create(&create, (void *)2);
	while(1) {
		drop_exec();
	}
	exit();
}
#endif
