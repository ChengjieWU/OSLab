#ifndef __PROC_H__
#define __PROC_H__

#include "semaphore.h"

int fork();
int getpid();
int exit();
int sleep(int);

semaphore *sem_open(int);
int sem_close(semaphore *);
int sem_wait(semaphore *);
int sem_post(semaphore *);

#endif
