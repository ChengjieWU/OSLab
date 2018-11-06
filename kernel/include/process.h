#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "x86.h"
#include "wthread.h"
#define KERNEL_STACK_SIZE 1
#define PCB_NUM 64					/* The maximum number of PCB terms is 64. */

#define fcbmax 10

enum STATE {
    PROCESS_EMPTY,
    PROCESS_RUNNING,
    PROCESS_READY,
    PROCESS_BLOCKED,
    PROCESS_DEAD,
    PROCESS_SEMAPHORE
};


typedef struct PCB {
	void* tf;		/* This term must be the first! The reason lies in do_syscall.s, using current to lacate trapframe! */
	int id;
    enum STATE state;
    uint8_t* kernelStackBottom; /* Allocate a page for kernel stack. */
	struct PCB* next;
	CR3 cr3;
	pid_t pid;
	pid_t parent;
	PDE* pgdir;
	uint32_t cpuTime;
	int sleepTime;
	wthread *thread;
	int fcb_index[fcbmax];
} PCB;

extern PCB idle;
extern PCB* current;
extern PCB* shell;

extern TSS tss;


#endif
