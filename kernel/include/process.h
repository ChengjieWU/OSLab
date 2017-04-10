#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "x86.h"
#define KERNEL_STACK_SIZE 1
#define PCB_NUM 64					/* The maximum number of PCB terms is 64. */

enum STATE {
    PROCESS_EMPTY,
    PROCESS_RUNNING,
    PROCESS_READY,
    PROCESS_BLOCKED,
    PROCESS_DEAD,
};


typedef struct PCB {
	void* tf;		/* ###### Due to unknown reason, this term must be the first! ###### */
	int id;
    enum STATE state;
    uint8_t* kernelStackMax; /* Allocate a page for kernel stack. */
	struct PCB* next;
	CR3 cr3;
	pid_t pid;
	pid_t parent;
	PDE* pgdir;
} PCB;

void init_PCB();


extern PCB idle;
extern PCB* current;
extern TSS tss;


#endif
