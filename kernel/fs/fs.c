#include "fs.h"
#include "string.h"

void readsect(void *dst, int offset);
void readsect_n(void *, int, int);
void readseg(unsigned char *pa, int count, int offset);

struct bitmap bmap;
struct dir root;
struct inode Inode[FILENUM];

unsigned char buffer[blocksize];

void init_fs()
{
	readsect_n(bmap.mask, BMAPOFFSET, BMAPBLOCK);
	readsect_n((uint8_t *)Inode, INODEOFFSET, FILENUM);
	readsect_n((uint8_t *)&root, ROOTOFFSET, 1);	
}

/* Blocksize is equal to sectorsize here! */
void read_a_sect_of_file(int block_num)
{
	int inode_tmp = root.entries[1].inode_offset;
	while (block_num >= blocksize / sizeof(uint32_t) - 1)
	{
		block_num -= (blocksize / sizeof(uint32_t) - 1);
		inode_tmp = Inode[inode_tmp].data_block_offsets[blocksize / sizeof(uint32_t) - 1];
	}
	readsect(buffer, Inode[inode_tmp].data_block_offsets[block_num]);
}


void read_file(unsigned char *start, int count, int offset)
{
	if (root.entries[1].file_size == 0) panic(0);
	
	int start_bias = offset % blocksize;
	int start_length = blocksize - start_bias;
	int start_block = offset / blocksize;
	int end_block = (offset + count) / blocksize;
	
	if (start_block == end_block)
	{
		read_a_sect_of_file(start_block);
		memcpy(start, buffer + start_bias, count);
	}
	else
	{
		read_a_sect_of_file(start_block);
		memcpy(start, buffer + start_bias, start_length);
		start += start_length;
		start_block += 1;
		count -= start_length;
		while (start_block < end_block)
		{
			read_a_sect_of_file(start_block);
			memcpy(start, buffer, blocksize);
			start += blocksize;
			start_block += 1;
			count -= blocksize;
		}
		read_a_sect_of_file(start_block);
		memcpy(start, buffer, count);
	}
}
