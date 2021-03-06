#include "process.h"
#include "irq.h"
#include "memory.h"

PCB idle;
PCB* current = &idle;
PCB* shell;
PCB pcb_pool[PCB_NUM];
PCB* pcb_free_list;
PCB* pcb_ready_list;
PCB* pcb_blocked_list;
int pid_pool;
bool hasBlocked;


uint32_t request_for_page();
PDE* init_updir();

void file_close_all();


void init_PCB()
{
	pcb_ready_list = NULL;
	pcb_blocked_list = NULL;
	pid_pool = -1;		/* The first process is 0. */
	hasBlocked = false;
	int i;
	for (i = 0; i < PCB_NUM; i++)
	{
		pcb_pool[i].tf = NULL;
		pcb_pool[i].id = i;
		pcb_pool[i].state = PROCESS_EMPTY;
		pcb_pool[i].kernelStackBottom = NULL;
		pcb_pool[i].pid = -1;
		pcb_pool[i].parent = -1;
		pcb_pool[i].pgdir = NULL;
		pcb_pool[i].cpuTime = 0;
		pcb_pool[i].sleepTime = 0;
		pcb_pool[i].thread = NULL;
		
		int j;
		for (j = 0; j < fcbmax; j++)
			pcb_pool[i].fcb_index[j] = -1;
		
		pcb_pool[i].next = pcb_free_list;
		pcb_free_list = &pcb_pool[i];
	}
}

/* Allocate a new PCB term. */
PCB* pcb_alloc()
{
	if (pcb_free_list == NULL) return NULL;
	PCB* p = pcb_free_list;
	pcb_free_list = pcb_free_list->next;
	return p;
}

/* Free a PCB. */
void pcb_free(PCB *pcb)
{
	if (pcb == NULL) panic("Trying to free an empty PCB!\n");
	pcb->tf = NULL;
	pcb->state = PROCESS_EMPTY;
	pcb->kernelStackBottom = NULL;
	pcb->pid = -1;
	pcb->parent = 1;
	pcb->pgdir = NULL;
	pcb->cpuTime = 0;
	pcb->sleepTime = 0;
	pcb->thread = NULL;
	
	/*###### Only for a process exit by itself! ######*/
	file_close_all();
	
	pcb->next = pcb_free_list;
	pcb_free_list = pcb;
}

void add_ready_list(PCB* pcb)		//add to tail
{
	pcb->next = NULL;
	pcb->state = PROCESS_READY;
	PCB* p = pcb_ready_list;
	if (p == NULL) pcb_ready_list = pcb;
	else
	{
		while (p->next != NULL) p = p->next;
		p->next = pcb;
	}
}

int ready_list_length()
{
	int len = 0;
	PCB *p = pcb_ready_list;;
	while (p != NULL) {p = p->next; len += 1;}
	return len;
}

PCB* pop_ready_list()				//pop the head
{
	bool overshell = ready_list_length() > 1;
	if (pcb_ready_list != NULL)
	{
		PCB* p = pcb_ready_list;
		pcb_ready_list = pcb_ready_list->next;
		p->next = NULL;
		//printk("%d\n", ready_list_length());
		if (overshell && p == shell)
		{
			//printk("%d\n", ready_list_length());
			add_ready_list(p);
			p = pop_ready_list();
		}
		return p;
	}
	else 
		return NULL;
}



/* Add current process to blocked list according to its sleep time. */
void add_blocked_list(PCB* pcb, uint32_t t)
{
	/*printk("before: ");		//for debug
	PCB *ta = pcb_blocked_list;
	for (; ta != NULL; ta = ta->next) printk("%d ", ta->pid);
	printk("\n");*/
	pcb->next = NULL;
	pcb->state = PROCESS_BLOCKED;
	if (pcb_blocked_list == NULL) 
	{
		pcb->sleepTime = t;
		pcb_blocked_list = pcb;
		hasBlocked = true;
	}
	else
	{
		PCB *p = pcb_blocked_list;
		uint32_t cut = p->sleepTime;
		if (t < cut)
		{
			p->sleepTime -= t;
			pcb->sleepTime = t;
			pcb->next = p;
			pcb_blocked_list = pcb;
		}
		else
		{
			PCB* q = p->next;
			while (p->next != NULL)
			{
				if (cut + q->sleepTime > t) break;
				cut += q->sleepTime;
				p = p->next;
				q = q->next;
			}
			if (q != NULL) q->sleepTime -= (t - cut);
			pcb->sleepTime = t - cut;
			pcb->next = q;
			p->next = pcb;
		}
	}
	/*printk("after: ");
	ta = pcb_blocked_list;
	for (; ta != NULL; ta = ta->next) printk("%d ", ta->pid);
	printk("\n");*/
}
PCB* pop_blocked_list()				//pop the head
{
	if (pcb_blocked_list != NULL)
	{
		PCB* p = pcb_blocked_list;
		pcb_blocked_list = pcb_blocked_list->next;
		p->next = NULL;
		if (pcb_blocked_list == NULL) hasBlocked = false;
		return p;
	}
	else 
		return NULL;
}

/* Create a new process. */
PCB* new_process()
{
	PCB* pcb = pcb_alloc();
	pcb->parent = pid_pool;
	pcb->pid = ++pid_pool;	
	pcb->pgdir = init_updir();
	pcb->next = NULL;
	/* ###### Restrict kernel stack to 4KB. ###### */
	pcb->kernelStackBottom = (uint8_t*)request_for_page();
	pcb->tf = pcb->kernelStackBottom + PAGE_SIZE;
	
	//printk("%x\t%x\n", pcb->kernelStackBottom, pcb->tf);
	
	add_ready_list(pcb);
	
	return pcb;
}

/* Load cr3. */
void load_process_memory(PCB *pcb)
{
	PDE* updir = pcb->pgdir;
	CR3 cr3;
	cr3.val = 0;
	cr3.page_directory_base = ((uint32_t)va_to_pa(updir)) >> 12;
	//printk("\n%x\n", cr3.val);
	write_cr3(&cr3);
}
/* Switch trap frames. */
void change_to_process(PCB* pcb)
{
	pcb->state = PROCESS_RUNNING;
	current = pcb;
	tss.esp0 = (uint32_t)current->tf;
	/*** for debug only ***/       
	/*printk("current process is No.%d\n", current->pid);
	PCB* p = pcb_ready_list;
	printk("\tready list: ");
	for (; p != NULL; p = p->next) printk("%d ", p->pid);
	printk("\n");*/
	
	//printk("tss.esp0 = %x\n", tss.esp0);
}


void schedule()
{	
	uint32_t esp0 = (uint32_t)current->tf;		//free space of TrapFrame
    esp0 += sizeof(struct TrapFrame);
    tss.esp0 = esp0;
}

