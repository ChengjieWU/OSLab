#include "process.h"

int close(int fd);

int pcb_fcb_add(int fd)
{
	int i;
	for (i = 0; i < fcbmax; i++)
		if (current->fcb_index[i] == -1) break;
	if (i == fcbmax) return -1;
	current->fcb_index[i] = fd;
	return 0;
}

bool pcb_fcb_test(int fd)
{
	bool ret = false;
	int i;
	for (i = 0; i < fcbmax; i++)
		if (current->fcb_index[i] == fd) ret = true;
	return ret;
}

int pcb_fcb_free(int fd)
{
	int i;
	for (i = 0; i < fcbmax; i++)
		if (current->fcb_index[i] == fd) break;
	if (i == fcbmax) return -1;
	current->fcb_index[i] = -1;
	return 0;
}

void file_close_all()
{
	int i;
	for (i = 0; i < fcbmax; i++)
		if (current->fcb_index[i] != -1)
			close(current->fcb_index[i]);
}

void copy_fcb(PCB *p, PCB *q)
{
	int i;
	for (i = 0; i < fcbmax; i++)
		p->fcb_index[i] = q->fcb_index[i];
}
