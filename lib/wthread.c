#include "types.h"
#include "wthread.h"

void wthread_join(wthread *thread)
{
	/* Each thread has a semaphore telling others whether is has finished. */
	/* The semaphore is initialized to 0. After its exit, the semaphore is posted to 1. */
	/* All the threads join the thread are actually waiting on this exact semaphore. */
	sem_wait(&thread->has_exited);
	sem_post(&thread->has_exited);
}

/* NOTE: wthread_create and wthread_exit are implemented in syscall.c directly. */
