#include "fs.h"

void readsect_n(void *, int, int);

struct bitmap bmap;
struct dir root;
struct inode Inode[FILENUM];

void init_fs()
{
	readsect_n(bmap.mask, BMAPOFFSET, BMAPBLOCK);
	readsect_n((uint8_t *)Inode, INODEOFFSET, FILENUM);
	readsect_n((uint8_t *)&root, ROOTOFFSET, 1);
	
	/*int i = 0;
	for (; i < 512 * 3; i++)
	{
		printk("%x", bmap.mask[i]);
	}
	uint8_t *tmp = (uint8_t *)Inode;
	for (i = 0; i < 512 * 8; i++)
	{
		printk("%x", tmp[i]);
	}
	tmp = (uint8_t *)&root;
	for (i = 0; i < 512; i++)
	{
		printk("%x", tmp[i]);
	}
	printk("\n");*/
}
