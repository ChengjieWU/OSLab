#include "semaphore.h"
#include "process.h"

void dropRun();
void add_ready_list(PCB* pcb);

semaphore sem_pool[num_Sem];

/* The init_Sem function is called by OS when booting in order to allow use of named semaphore. */
void init_Sem()
{
	int i;
	for (i = 0; i < num_Sem; i++) {
		sem_pool[i].value = 0;
		sem_pool[i].id = i;
		sem_pool[i].wait_queue = NULL;
		sem_pool[i].use = false;
		sem_pool[i].cited = 0;
	}
}

/* The following two are P and V. */
int sem_wait_kernel(semaphore *sem)
{
	/* First test whether the semaphore is still in use. */
	if (sem->use == false) return -1;
	if (sem->value > 0) {
		sem->value--;
	}
	else {
		/* If sem->value <= 0, current thread should be blocked on the semaphore. */
		current->next = (PCB *)sem->wait_queue;
		sem->wait_queue = (void *)current;
		/* Processes in wait_queue need a status too! - implemented in dropRun() function. */
		/* Allow other threads to run. */
		dropRun();
	}
	return 0;
}
int sem_post_kernel(semaphore *sem)
{
	/* First test whether the semaphore is still in use. */
	if (sem->use == false) return -1;
	if (sem->value > 0 || sem->wait_queue == NULL) {
		sem->value++;
	}
	else {
		/* If sem->value == 0 and there are threads waiting, then wake up the first waiting thread. */
		PCB *p = (PCB *)sem->wait_queue;
		PCB *q = NULL;
		while (p->next != NULL) {
			q = p;
			p = p->next;
		}
		if (q != NULL) q->next = NULL;
		else sem->wait_queue = NULL;
		/* Wake it up means adding the thread to the ready list. */
		add_ready_list(p);
	}
	return 0;
}

/* The following two functions are only used in named semaphores between processes. */
semaphore *sem_open_kernel(int id, int value)
{	
	/* Open a closed semaphore or overwrite an eixsting semaphore. */
	if (sem_pool[id].use == false) sem_pool[id].use = true;
	sem_pool[id].cited++;
	sem_pool[id].value = value;
	return &sem_pool[id];
}
int sem_close_kernel(semaphore *sem)
{
	if (sem->use == true && --sem->cited == 0) sem->use = false;
	return 0;
}


/* The following functions are used in unnamed semaphores. */
int sem_init_kernel(semaphore *sem, int value)
{
	sem->value = value;
	sem->wait_queue = NULL;
	sem->use = true;
	/* sem->id and sem->cited are not used in unnamed semaphore. */
	return 0;
}
int sem_destroy_kernel(semaphore *sem)
{
	sem->use = false;
	sem->wait_queue = NULL;
	sem->value = 0;
	return 0;
}
