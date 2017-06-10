#include "fs.h"

void readseg(unsigned char*,int,int);
void readsect(void *dst, int offset);

struct bitmap bmap;

void init_fs()
{
	readsect(bmap.mask, BMAPOFFSET);
	int i;
	for (i = 0; i < 512; i++)
		printk("%x", bmap.mask[i]);
	printk("\n");
}
