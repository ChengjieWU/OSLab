#include "fs.h"
#include "string.h"
#include "disk.h"

extern unsigned char buffer[blocksize];
extern struct bitmap bmap;
extern struct dir root;
extern struct inode Inode[FILENUM];
extern struct FCB fcb[FCBNUM];
extern struct FCB *fcb_free_list;

/* Blocksize is equal to sectorsize here! */
/* Note that in the following two functions, legality is not checked! */
void read_a_sect_of_file(int block_num, int inode_offset)
{
	int inode_tmp = inode_offset;
	while (block_num >= blocksize / sizeof(uint32_t) - 1)
	{
		block_num -= (blocksize / sizeof(uint32_t) - 1);
		inode_tmp = Inode[inode_tmp].data_block_offsets[blocksize / sizeof(uint32_t) - 1];
	}
	readsect(buffer, Inode[inode_tmp].data_block_offsets[block_num]);
}
void write_a_sect_of_file(int block_num, int inode_offset)
{
	int inode_tmp = inode_offset;
	while (block_num >= blocksize / sizeof(uint32_t) - 1)
	{
		block_num -= (blocksize / sizeof(uint32_t) - 1);
		inode_tmp = Inode[inode_tmp].data_block_offsets[blocksize / sizeof(uint32_t) - 1];
	}
	writesect(buffer, Inode[inode_tmp].data_block_offsets[block_num]);
}
void read_a_part_of_file(unsigned char *start, int inode_offset, int offset, int count)
{
	if (count <= 0) return;	//for safe and sound
	/* Note that currently, inode_offset is the inode index of the first inode of the file. */
	/* offset is the offset within the whole file, and count is the number of bytes read. */
	int start_bias = offset % blocksize;
	int start_length = blocksize - start_bias;
	int start_block = offset / blocksize;
	int end_block = (offset + count - 1) / blocksize;/***************/
	
	if (start_block == end_block)
	{
		read_a_sect_of_file(start_block, inode_offset);
		memcpy(start, buffer + start_bias, count);
	}
	else
	{
		read_a_sect_of_file(start_block, inode_offset);
		memcpy(start, buffer + start_bias, start_length);
		start += start_length;
		start_block += 1;
		count -= start_length;
		while (start_block < end_block)
		{
			read_a_sect_of_file(start_block, inode_offset);
			memcpy(start, buffer, blocksize);
			start += blocksize;
			start_block += 1;
			count -= blocksize;
		}
		read_a_sect_of_file(start_block, inode_offset);
		memcpy(start, buffer, count);
	}
}
void write_a_part_of_file(unsigned char *start, int inode_offset, int offset, int count)
{
	if (count <= 0) return;	//for safe and sound
	int start_bias = offset % blocksize;
	int start_length = blocksize - start_bias;
	int start_block = offset / blocksize;
	/* Important note: when calculating end_block in write, we should minus 1. In read, we should not. */
	/* The write function also. */
	/* Why read not? Because even not minus one, the last sector read will read 0 bytes, which is ok. */
	int end_block = (offset + count - 1) / blocksize;
	
	if (start_block == end_block)
	{
		read_a_sect_of_file(start_block, inode_offset);
		memcpy(buffer + start_bias, start, count);
		write_a_sect_of_file(start_block, inode_offset);
	}
	else
	{
		read_a_sect_of_file(start_block, inode_offset);
		memcpy(buffer + start_bias, start, start_length);
		write_a_sect_of_file(start_block, inode_offset);
		start += start_length;
		start_block += 1;
		count -= start_length;
		while (start_block < end_block)
		{
			memcpy(buffer, start, blocksize);
			write_a_sect_of_file(start_block, inode_offset);
			start += blocksize;
			start_block += 1;
			count -= blocksize;
		}
		read_a_sect_of_file(start_block, inode_offset);
		memcpy(buffer, start, count);
		write_a_sect_of_file(start_block, inode_offset);
	}
}
