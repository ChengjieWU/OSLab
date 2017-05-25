#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "types.h"

#define num_Sem 20

typedef struct semaphore {
	int value;
	int id;
	void *wait_queue;
	bool empty;
	int cited;
} semaphore;

semaphore *sem_open(int);
int sem_close(semaphore *);
int sem_wait(semaphore *);
int sem_post(semaphore *);
int sem_init(semaphore *, int);
int sem_destroy(semaphore *);


#endif
