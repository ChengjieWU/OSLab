#ifndef __WTHREAD_H__
#define __WTHREAD_H__

#include "semaphore.h"

typedef struct wthread {
	semaphore has_exited;
} wthread;

int wthread_create(wthread *, void *, void *);
int wthread_exit();
void wthread_join(wthread *);



#endif
