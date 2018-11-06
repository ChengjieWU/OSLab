#include "process.h"
#include "string.h"
#include "irq.h"
#include "memory.h"
#include "wthread.h"
#include "elf.h"

PCB* new_process();
void change_to_process(PCB *);
void load_process_memory(PCB *);
void add_ready_list(PCB* );
PCB* pop_ready_list();
void pcb_free(PCB *);
void page_remove_phy(physaddr_t);
void add_blocked_list(PCB*, uint32_t);
PCB* pop_blocked_list();

/* These two are used to implement wthread_join. */
int sem_init_kernel(semaphore *, int);
int sem_post_kernel(semaphore *);

/* copy fcbs */
void copy_fcb(PCB *p, PCB *q);

/* load programes */
int find_file(char *name);
int load_program(int index, unsigned char *start, int count, int offset);

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
	
	/* Copy owned fcbs. */
	copy_fcb(child, parent);
	
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

void wthread_create_kernel(void *func, void *arg, wthread *thread)
{
	/* Add current process to ready list. */
	add_ready_list(current);
	
	/* Create a new process. */
	PCB* child = new_process();
	PCB* parent = current;
	child->parent = parent->pid;
	PDE* cpdir = child->pgdir;
	PDE* ppdir = parent->pgdir;
	
	/* Copy owned fcbs. */
	copy_fcb(child, parent);
	
	/* Copy kernel stack from parent to its new process. */
	memcpy(child->kernelStackBottom, parent->kernelStackBottom, PAGE_SIZE);
	
	child->tf = (void *)((uint32_t)child->kernelStackBottom + ((uint32_t)parent->tf - (uint32_t)parent->kernelStackBottom));
	
	/* Set return value of the fork() function. 0 for child and child's pid for parent. */
	((TrapFrame *)(child->tf))->eax = 0;
	((TrapFrame *)(parent->tf))->eax = child->pid;
	
	/* THREAD: Set %eip to the objective function. */
	((TrapFrame *)(child->tf))->eip = (uint32_t)func;
	
	/* THREAD: This deals with storing information of threads. */
	child->thread = thread;
	sem_init_kernel(&child->thread->has_exited, 0);
	
	uint32_t pdir_idx;
	
	/* THREAD: Shallow copy page directory, page table and physical frames of other components.*/
	/* In my implementation, thread won't increment the page cited number. */
	/* Any illegal operations, such as the main process exiting first, will lead to unexpected result. */
	for (pdir_idx = 0; pdir_idx < USER_STACK_TOP / PD_SIZE; pdir_idx++)
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
				/* Shallow copy. */
			}
		}
	}
	
	/* Deep copy page directory, page table and physical frames of user stack. */
	for (pdir_idx = USER_STACK_TOP / PD_SIZE; pdir_idx < KOFFSET / PD_SIZE; pdir_idx++)
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
	
	/* THREAD: Switch to child memory temporarily and load the argument! */
	/* The first argument is at %ebp + 0x8. However, we do not have a call procedure. So 0x4 must be added. */
	load_process_memory(child);
	*(uint32_t *)(((TrapFrame *)(child->tf))->esp + 0x4) = (uint32_t)arg;
	
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

void wthread_exit_kernel()
{
	PDE* pgdir = current->pgdir;
	uint32_t pdir_idx;
	for (pdir_idx = USER_STACK_TOP / PD_SIZE; pdir_idx < KOFFSET / PD_SIZE; pdir_idx++)
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
	
	/* To tell other threads that I have exited. */
	sem_post_kernel(&current->thread->has_exited);
	
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

/* The below 2 are different! */
/* dropRun is used in semaphores waiting list */
void dropRun()
{
	current->cpuTime = 0;
	current->state = PROCESS_SEMAPHORE;
	PCB *pcb = pop_ready_list();
	if (pcb == NULL) panic("\nThere are no processes. Machine stops!\n");
	load_process_memory(pcb);
	change_to_process(pcb);
}
/* drop_exec_kernel is used in drop_exec system call. */
void drop_exec_kernel()
{
	current->cpuTime = 0;
	/*!!*/add_ready_list(current);
	PCB *pcb = pop_ready_list();
	if (pcb == NULL) panic("\nThere are no processes. Machine stops!\n");
	load_process_memory(pcb);
	change_to_process(pcb);
}



/*###### ######*/

uint8_t exec_buf[4096];

void exec_kernel(char *name)
{	
	/* Test if such program is valid. If it is not, set return value -1. */
	int index = find_file(name);
	if (index == -1)
	{
		((TrapFrame *)(current->tf))->eax = -1;
		return;
	}
	
	struct Elf *elf;
	struct Proghdr *ph, *eph;
	unsigned char *pa, *i;
	
	elf = (struct Elf*)exec_buf;
	
	load_program(index, (unsigned char*)elf, 4096, 0);
	
	const uint32_t elf_magic = 0x464c457f;
	uint32_t *p_magic = (void *)exec_buf;
	if (elf_magic != *p_magic)
	{
		((TrapFrame *)(current->tf))->eax = -1;
		return;
	}
	
	
	
	PCB* pn = new_process();
	pn->parent = current->parent;
	pn->pid = current->pid;
	copy_fcb(pn, current);
	
	
	/* Copy kernel stack. */
	memcpy(pn->kernelStackBottom, current->kernelStackBottom, PAGE_SIZE);
	pn->tf = (void *)((uint32_t)pn->kernelStackBottom + ((uint32_t)current->tf - (uint32_t)current->kernelStackBottom));
	
	unsigned pdir_idx;
	
	/* copy page directory, page table and physical frames. */
	PDE* cpdir = pn->pgdir;
	PDE* ppdir = current->pgdir;
	
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
					memcpy(cc, pc, PAGE_SIZE);
				}
			}
		}
	}
	
	/* Delete the original process. */
	PDE* pgdir = current->pgdir;
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
	pcb_free(current);
	
	/* ReLoad the new program. */
	current = pn;
	load_process_memory(current);


	ph = (struct Proghdr*)((char *)elf + elf->e_phoff);
	eph = ph + elf->e_phnum;
	for(; ph < eph; ph ++) {
		pa = (unsigned char*)ph->p_pa;
		load_program(index, pa, ph->p_filesz, ph->p_offset);
		for(i = pa + ph->p_filesz; i < pa + ph->p_memsz; *i ++ = 0);
	}
	
	((TrapFrame *)(current->tf))->eip = elf->e_entry;
	
	PCB* pcb = pop_ready_list();
	load_process_memory(pcb);
	change_to_process(pcb);
}


