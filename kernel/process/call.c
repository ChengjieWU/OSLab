#include "process.h"
#include "string.h"
#include "irq.h"
#include "memory.h"

PCB* new_process();
void change_to_process(PCB *);
void load_process_memory(PCB *);
void add_ready_list(PCB* );
PCB* pop_ready_list();
void pcb_free(PCB *);
void page_remove_phy(physaddr_t);
void add_blocked_list(PCB*, uint32_t);
PCB* pop_blocked_list();


void fork_kernel()
{
	/* Add current process to ready list. */
	add_ready_list(current);
	
	/* Create a new process. */
	PCB* child = new_process();
	PCB* parent = current;
	child->parent = parent->pid;
	PDE* cpdir = child->pgdir;
	PDE* ppdir = parent->pgdir;
	
	/* Copy kernel stack from parent to its new process. */
	memcpy(child->kernelStackBottom, parent->kernelStackBottom, PAGE_SIZE);
	
	child->tf = (void *)((uint32_t)child->kernelStackBottom + ((uint32_t)parent->tf - (uint32_t)parent->kernelStackBottom));
	
	/* Set return value of the fork() function. 0 for child and child's pid for parent. */
	((TrapFrame *)(child->tf))->eax = 0;
	((TrapFrame *)(parent->tf))->eax = child->pid;
	
	/* Deep copy page directory, page table and physical frames. */
	uint32_t pdir_idx;
	for (pdir_idx = 0; pdir_idx < KOFFSET / PD_SIZE; pdir_idx++)
	{
		cpdir[pdir_idx] = ppdir[pdir_idx];
		if (cpdir[pdir_idx].present)
		{
			PTE* cptable = (PTE*)request_for_page();
			make_pde(&cpdir[pdir_idx], va_to_pa(cptable));
			PTE* pptable = (PTE*)va_pte(&ppdir[pdir_idx]);
			uint32_t ptable_idx;
			for (ptable_idx = 0; ptable_idx < NR_PTE; ptable_idx++)
			{
				cptable[ptable_idx] = pptable[ptable_idx];
				if (cptable[ptable_idx].present)
				{
					uint8_t *cc = (uint8_t *)request_for_page();
					make_pte(&cptable[ptable_idx], va_to_pa(cc));
					uint8_t *pc = (uint8_t *)va_byte(&pptable[ptable_idx]);
					/* deep copy */
					memcpy(cc, pc, PAGE_SIZE);
				}
			}
		}
	}
	
	/* Select a process in the ready list to run. */
	PCB* pcb = pop_ready_list();
	load_process_memory(pcb);
	change_to_process(pcb);
}

int get_pid()
{
	return current->pid;
}

void exit_kernel()
{	
	PDE* pgdir = current->pgdir;
	
	/* Remove all physical frames. */
	uint32_t pdir_idx;
	for (pdir_idx = 0; pdir_idx < KOFFSET / PD_SIZE; pdir_idx++)
	{
		if (pgdir[pdir_idx].present)
		{
			PTE* pgtable = (PTE *)va_pte(&pgdir[pdir_idx]);
			physaddr_t pa = (physaddr_t)pgdir[pdir_idx].page_frame << 12;
			uint32_t ptable_idx;
			for (ptable_idx = 0; ptable_idx < NR_PTE; ptable_idx++)
			{
				if (pgtable[ptable_idx].present)
					page_remove(pgdir, (void *)va_byte(&pgtable[ptable_idx]));
			}
			page_remove_phy(pa);
		}
	}
	
	/* Free the PCB. */
	pcb_free(current);
	
	/* Select a process in the ready list to run. */
	PCB* pcb = pop_ready_list();
	if (pcb == NULL) panic("\nThere are no processes. Machine stops!\n");
	load_process_memory(pcb);
	change_to_process(pcb);
	
}

/* Change processes due to CPU time. */
void timeChange()
{
	current->cpuTime = 0;
	add_ready_list(current);
	PCB* pcb = pop_ready_list();
	if (pcb == NULL) panic("\nThere are no processes. Machine stops!\n");
	load_process_memory(pcb);
	change_to_process(pcb);
}
/* Sleep the current process. */
void sleep_kernel(int hl)
{
	current->cpuTime = 0;
	add_blocked_list(current, hl);
	PCB* pcb = pop_ready_list();
	if (pcb == NULL) panic("\nThere are no processes. Machine stops!\n");
	load_process_memory(pcb);
	change_to_process(pcb);
}
/* Wake up the head of the blocked list. */
void wakeup()
{
	PCB *p = pop_blocked_list();
	p->sleepTime = 0;
	add_ready_list(p);
}

void dropRun()
{
	current->cpuTime = 0;
	PCB *pcb = pop_ready_list();
	if (pcb == NULL) panic("\nThere are no processes. Machine stops!\n");
	load_process_memory(pcb);
	change_to_process(pcb);
}
