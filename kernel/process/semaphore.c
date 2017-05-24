#include "semaphore.h"
#include "process.h"

void dropRun();
void add_ready_list(PCB* pcb);

semaphore sem_pool[num_Sem];

void init_Sem()
{
	int i;
	for (i = 0; i < num_Sem; i++) {
		sem_pool[i].value = 0;
		sem_pool[i].id = i;
		sem_pool[i].wait_queue = NULL;
		sem_pool[i].empty = true;
		sem_pool[i].cited = 0;
	}
}

semaphore *sem_open_kernel(int id)
{	
	if (sem_pool[id].empty == true) sem_pool[id].empty = false;
	sem_pool[id].cited++;
	
	/****** Currently, the initial value is set to 1, and it needs modifying. ******/
	sem_pool[id].value = 3;
	
	return &sem_pool[id];
}

int sem_close_kernel(semaphore *sem)
{
	if (--sem->cited == 0) sem->empty = true;
	return 0;
}

int sem_wait_kernel(semaphore *sem)
{
	if (sem->value > 0) {
		sem->value--;
		return 0;
	}
	else {
		current->next = (PCB *)sem->wait_queue;
		sem->wait_queue = (void *)current;
		
		dropRun();
		return 0;
	}
}

int sem_post_kernel(semaphore *sem)
{
	if (sem->value > 0 || sem->wait_queue == NULL) {
		sem->value++;
		return 0;
	}
	else {
		PCB *tmp = (PCB *)sem->wait_queue;
		sem->wait_queue = tmp->next;
		add_ready_list(tmp);
		return 0;
	}
}
