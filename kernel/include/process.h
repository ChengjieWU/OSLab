#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "x86.h"
#define KERNEL_STACK_SIZE PAGE_SIZE
#define PCB_NUM 64					/* The maximum number of PCB terms is 64. */

enum STATE {
    TASK_EMPTY,
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_DEAD,
};


typedef struct PCB {
    void *tf;
    enum STATE state;
    char kernelStack[KERNEL_STACK_SIZE];
	struct PCB* next;
	CR3 cr3;
	pid_t pid;
	pid_t parent;
} PCB;



extern PCB idle;
extern PCB* current;
extern TSS tss;


#endif
