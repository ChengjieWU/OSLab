#include "types.h"
#include "printf.h"
#include "time.h"
#include "string.h"

#include "proc.h"
#include "wthread.h"

/* There are 2 test cases in this file.*/

//#define compete_for_limited_resource_test
#define producer_and_consumer



/* TEST CASE 1: producer and consumer */

#ifdef producer_and_consumer

const int N = 100;
const int T = 100;
int buf[1000];
int count;

semaphore mutex;
semaphore empty;
semaphore full;
wthread p1, p2;
wthread c1, c2;

int produce_item()
{
	buf[count] = count;
	count++;
	return buf[count - 1];
}

int consume_item()
{
	return buf[--count];
}

void producer(void *arg)
{
	int i = T;
	while(i--) {
		sem_wait(&empty);
		sem_wait(&mutex);
		printf("producer: %d produce %d\n", (int)arg, produce_item());
		sem_post(&mutex);
		sem_post(&full);
		drop_exec();
	}
	wthread_exit();
}

void consumer(void *arg)
{
	int i = T;
	while(i--) {
		sem_wait(&full);
		sem_wait(&mutex);
		printf("consumer: %d consume %d\n", (int)arg, consume_item());
		sem_post(&mutex);
		sem_post(&empty);
		drop_exec();
	}
	wthread_exit();
}

void test_main()
{
	count = 0;
	memset(buf, 0, sizeof buf);
	
	sem_init(&mutex, 1);
	sem_init(&empty, N);
	sem_init(&full, 0);
	wthread_create(&p1, &producer, (void *)1);
	wthread_create(&p2, &producer, (void *)2);
	wthread_create(&c1, &consumer, (void *)1);
	wthread_create(&c2, &consumer, (void *)2);
	wthread_join(&p1);
	wthread_join(&p2);
	wthread_join(&c1);
	wthread_join(&c2);
	sem_destroy(&mutex);
	sem_destroy(&empty);
	sem_destroy(&full);
	exit();
}
#endif



/* TEST CASE 2: multiple processes or threads compete for limited resources */

#ifdef compete_for_limited_resource_test

semaphore sem;
wthread T[16];
void create(void* arg)
{
	int t = getpid();
	int i;
	for (i = 0; i < 10000; i++) {
		sem_wait(&sem);
		if (i == 9999) printf("Thread No.%d OK!\n", t);
		sem_post(&sem);
	}
	wthread_exit();
}
void test_main()
{
	printf("*******************************************************\n");
	printf("Test unnamed semaphores between threads:\n");
	sem_init(&sem, 8);
	int i;
	for (i = 0; i < 16; i++) wthread_create(&T[i], &create, (void *)i);
	for (i = 0; i < 16; i++) wthread_join(&T[i]);
	sem_destroy(&sem);
	printf("*********************PARTING LINE**********************\n");
	printf("Test named semaphores between processes:\n");
	semaphore *sem = sem_open(0, 3);
	for (i = 0; i < 4; i++) fork();
	int t = getpid();
	for (i = 0; i < 10000; i++)
	{
		sem_wait(sem);
		if (i == 9999) printf("Process No.%d OK!\n", t);
		sem_post(sem);
	}
	exit();
}

#endif
