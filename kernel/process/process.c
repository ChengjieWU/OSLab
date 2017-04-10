#include "process.h"
#include "irq.h"

PCB idle;
PCB* current = &idle;
PCB pcb_pool[PCB_NUM];
PCB* pcb_free_list;

uint32_t request_for_page();
PDE* init_updir();

void init_PCB()
{
	int i;
	for (i = 0; i < PCB_NUM; i++)
	{
		pcb_pool[i].tf = NULL;
		pcb_pool[i].id = i;
		pcb_pool[i].state = PROCESS_EMPTY;
		pcb_pool[i].kernelStackMax = NULL;
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


PCB* go_schedule()
{
	PCB* pcb = pcb_alloc();
	pcb->state = PROCESS_READY;
	pcb->pid = 0;	/* The first process. */
	pcb->parent = -1;
	pcb->pgdir = init_updir();
	pcb->next = NULL;
	pcb->kernelStackMax = (uint8_t*)request_for_page();
	pcb->tf = pcb->kernelStackMax + PAGE_SIZE;
	return pcb;
}

void schedule()
{
	uint32_t esp0 = (uint32_t)current->tf;		//free space of TrapFrame
    esp0 += sizeof(struct TrapFrame);
    tss.esp0 = esp0;
}
