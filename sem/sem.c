#include "types.h"
#include "printf.h"
#include "time.h"
#include "string.h"

#include "proc.h"
#include "wthread.h"

//#define named
//#define unnamed
#define proacon


#ifdef proacon

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
	/*while(1) {
		drop_exec();
	}*/
	wthread_join(&p1);
	wthread_join(&p2);
	wthread_join(&c1);
	wthread_join(&c2);
	exit();
}
#endif



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
