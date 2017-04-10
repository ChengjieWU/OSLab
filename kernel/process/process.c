#include "string.h"
#include "process.h"
#include "irq.h"

PCB idle;
PCB* current = &idle;
PCB pcb_pool[PCB_NUM];
PCB* pcb_free_list;

void init_PCB()
{
	int i;
	for (i = 0; i < PCB_NUM; i++)
	{
		pcb_pool[i].tf = NULL;
		pcb_pool[i].id = i;
		pcb_pool[i].state = TASK_EMPTY;
		memset(pcb_pool[i].kernelStack, 0, KERNEL_STACK_SIZE);
		pcb_pool[i].pid = -1;
		pcb_pool[i].parent = -1;
		pcb_pool[i].pgdir = NULL;
		pcb_pool[i].next = pcb_free_list;
		pcb_free_list = &pcb_pool[i];
	}
}

PCB* pcb_alloc()
{
	if (pcb_free_list == NULL) return NULL;
	PCB* p = pcb_free_list;
	pcb_free_list = pcb_free_list->next;
	return p;
}

void schedule()
{
	uint32_t esp0 = (uint32_t)current->tf;		//free space of TrapFrame
    esp0 += sizeof(struct TrapFrame);
    tss.esp0 = esp0;
}
