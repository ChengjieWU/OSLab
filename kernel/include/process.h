#include "x86/memory.h"

#define KSTACK_SIZE 4096

typedef struct PCB {
    void *tf;
	char kstack[KSTACK_SIZE];
} PCB;
