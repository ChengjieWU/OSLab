#include "process.h"
#include "string.h"
#include "irq.h"
#include "memory.h"

PCB* new_process();
void change_to_process(PCB* pcb);
void load_process_memory(PCB *pcb);

void fork_kernel()
{
	PCB* child = new_process();
	PCB* parent = current;
	child->parent = parent->pid;
	PDE* cpdir = child->pgdir;
	PDE* ppdir = parent->pgdir;
	
	memcpy(child->kernelStackBottom, parent->kernelStackBottom, PAGE_SIZE);
	
	child->tf = (void *)((uint32_t)child->kernelStackBottom + ((uint32_t)parent->tf - (uint32_t)parent->kernelStackBottom));
	
	((TrapFrame *)(child->tf))->eax = 0;
	((TrapFrame *)(parent->tf))->eax = child->pid;
	
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
	
	load_process_memory(child);
	change_to_process(child);
}

int _get_pid()
{
	return current->pid;
}
