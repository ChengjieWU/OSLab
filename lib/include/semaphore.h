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


#endif
