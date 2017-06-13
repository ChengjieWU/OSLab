#include "fs.h"

/* The content is a copy of a period of fs.c in kernel, supporting read_file function. */

void readsect(void *dst, int offset);
void readsect_n(void *dst, int offset, int num);

unsigned char buffer[blocksize];
struct bitmap bmap;
struct dir root;
struct inode Inode[FILENUM];

void prepare_fs()
{
	readsect_n(bmap.mask, BMAPOFFSET, BMAPBLOCK);
	readsect_n((unsigned char *)Inode, INODEOFFSET, FILENUM);
	readsect_n((unsigned char *)&root, ROOTOFFSET, 1);
}


void *
memcpy(void *dst, const void *src, int n)
{
	const char *s;
	char *d;
	s = src;
	d = dst;
	if (s < d && s + n > d) {
		s += n;
		d += n;
		while (n-- > 0)
			*--d = *--s;
	} else
		while (n-- > 0)
			*d++ = *s++;
	return dst;
}

int
strcmp(const char *p, const char *q)
{
	while (*p && *p == *q)
		p++, q++;
	return (int) ((unsigned char) *p - (unsigned char) *q);
}

void read_a_sect_of_file(int block_num, int inode_offset)
{
	int inode_tmp = inode_offset;
	while (block_num >= blocksize / sizeof(unsigned) - 1)
	{
		block_num -= (blocksize / sizeof(unsigned) - 1);
		inode_tmp = Inode[inode_tmp].data_block_offsets[blocksize / sizeof(unsigned) - 1];
	}
	readsect(buffer, Inode[inode_tmp].data_block_offsets[block_num]);
}

void read_a_part_of_file(unsigned char *start, int inode_offset, int offset, int count)
{
	if (count <= 0) return;	//for safe and sound
	/* Note that currently, inode_offset is the inode index of the first inode of the file. */
	/* offset is the offset within the whole file, and count is the number of bytes read. */
	int start_bias = offset % blocksize;
	int start_length = blocksize - start_bias;
	int start_block = offset / blocksize;
	int end_block = (offset + count - 1) / blocksize;
	
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

void read_kernel_file(unsigned char *start, int count, int offset)
{
	int i;
	for (i = 0; i < blocksize / sizeof(struct dirent); i++)
		if (strcmp(root.entries[i].filename, "kernel.bin\0") == 0) break;
	int inode_offset = root.entries[i].inode_offset;
	if (count <= 0) return;	//for safe and sound
	read_a_part_of_file(start, inode_offset, offset, count);
}
