#include "types.h"
#include "printf.h"
#include "time.h"
#include "string.h"

#include "proc.h"
#include "wthread.h"

/**************************README!!!******************************/

/* There are 2 test cases in this file.*/
/* Switch between the 2 test cases using comment!!! */

//#define compete_for_limited_resource_test
#define producer_and_consumer

/*****************************************************************/




/* TEST CASE 1: producer and consumer */

#ifdef producer_and_consumer

/* Parameters */
#define T 10
#define N 4
#define PN 2
#define CN 2

int buf[N];
int count;

semaphore mutex;
semaphore empty;
semaphore full;

wthread p[PN];
wthread c[CN];

/* Halt funciton is used to make the whole process slower. */
void halt()
{
	int i;
	for (i = 0; i < 0x01ffffff; i++);
}
int produce_item(int i)
{
	halt();
	buf[count++] = i;
	return buf[count - 1];
}
int consume_item()
{
	halt();
	return buf[--count];
}
/* Producer thread. */
void producer(void *arg)
{
	int i;
	for (i = 0; i < T; i++) {
		sem_wait(&empty);
		sem_wait(&mutex);
		printf("\033[1;30;47mProducer No.\033[1;31;47m%d\033[1;30;47m puts data No.\033[1;31;47m%d\033[1;30;47m into the buffer\033[0m\n", (int)arg, produce_item(PN * i + (int)arg));
		sem_post(&mutex);
		sem_post(&full);
	}
	wthread_exit();
}
/* Consumer thread. */
void consumer(void *arg)
{
	int i;
	for (i = 0; i < T; i++) {
		sem_wait(&full);
		sem_wait(&mutex);
		printf("\t\033[1;34;47mConsumer No.\033[1;31;47m%d\033[1;34;47m withdraws data No.\033[1;31;47m%d\033[1;34;47m from the buffer\033[0m\n", (int)arg, consume_item());
		sem_post(&mutex);
		sem_post(&empty);
	}
	wthread_exit();
}
/* Main process. */
void test_main()
{
	count = 0;
	memset(buf, 0, sizeof buf);
	
	sem_init(&mutex, 1);
	sem_init(&empty, N);
	sem_init(&full, 0);
	
	int i;
	for (i = 0; i < PN; i++) {
		wthread_create(&p[i], &producer, (void *)i);
	}
	for (i = 0; i < CN; i++) {
		wthread_create(&c[i], &consumer, (void *)i);
	}
	for (i = 0; i < PN; i++) {
		wthread_join(&p[i]);
	}
	for (i = 0; i < CN; i++) {
		wthread_join(&c[i]);
	}
	
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
