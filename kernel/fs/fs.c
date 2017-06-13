#include "fs.h"
#include "string.h"
#include "disk.h"

unsigned char buffer[blocksize];
struct bitmap bmap;
struct dir root;
struct inode Inode[FILENUM];
struct FCB fcb[FCBNUM];
struct FCB *fcb_free_list;

void init_fs()
{
	readsect_n(bmap.mask, BMAPOFFSET, BMAPBLOCK);
	readsect_n((uint8_t *)Inode, INODEOFFSET, FILENUM);
	readsect_n((uint8_t *)&root, ROOTOFFSET, 1);
	fcb_free_list = NULL;
	int i;
	/* FCB 0, 1, 2 are reserved. */
	for (i = 3; i < FCBNUM; i++)
	{
		fcb[i].index = i;
		fcb[i].inode_offset = 0;
		fcb[i].file_size = 0;
		fcb[i].offset = 0;
		fcb[i].state = FS_CLOSE;
		fcb[i].next = fcb_free_list;
		fcb_free_list = &fcb[i];
		fcb[i].direntry = NULL;
	}
}
void fs_writeback()
{
	writesect_n(bmap.mask, BMAPOFFSET, BMAPBLOCK);
	writesect_n((uint8_t *)Inode, INODEOFFSET, FILENUM);
	writesect_n((uint8_t *)&root, ROOTOFFSET, 1);
}


int fcb_allocate()
{
	if (fcb_free_list == NULL) return -1;
	struct FCB *p = fcb_free_list;
	fcb_free_list = fcb_free_list->next;
	return p->index;
}

bool fcb_free(int fd)
{
	if (fd < 0 || fd >= FCBNUM) return false;
	fcb[fd].next = fcb_free_list;
	fcb_free_list = &fcb[fd];
	return true;
}

int request_free_root_dirent()
{
	int i;
	for (i = 0; i < blocksize / sizeof(struct dirent); i++)
	{
		if (root.entries[i].file_size == 0) break;
	}
	if (i == blocksize / sizeof(struct dirent)) panic("No free root directory entries!\n");
	return i;
}
/* Return Inode offset. */
uint32_t request_free_inode()
{
	int j;
	for (j = 0; j < FILENUM; j++)
	{
		if (Inode[j].data_block_offsets[0] == 0) break;
	}
	if (j == FILENUM) panic("No free inodes!\n");
	memset(buffer, 0, blocksize);
	writesect(buffer, j + INODEOFFSET);
	return j;
}
/* Return fd. If don't open it, return -1. */
int open(const char *pathname, enum FS_STATE state)
{
	int i;
	for (i = 0; i < blocksize / sizeof(struct dirent); i++)
		if (strcmp(root.entries[i].filename, pathname) == 0) break;
	if (i == blocksize / sizeof(struct dirent))
	{
		i = request_free_root_dirent();
		strcpy(root.entries[i].filename, pathname);
		root.entries[i].file_size = 0;
		root.entries[i].inode_offset = request_free_inode();
	}
	else if (state == FS_CLOSE) return -1;
	int fd = fcb_allocate();
	if (fd == -1) panic("No free fcb!\n");
	fcb[fd].state = state;
	fcb[fd].offset = 0;
	fcb[fd].inode_offset = root.entries[i].inode_offset;
	fcb[fd].file_size = root.entries[i].file_size;
	fcb[fd].direntry = &root.entries[i];
	fs_writeback();
	return fd;
}

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
uint32_t request_free_data_block()
{
	int byte;
	for (byte = 0; byte < BMAPTERM; byte++)
	{
		if (~bmap.mask[byte] == 0) continue;
		int bit;
		for (bit = 0; bit < 8; bit++)
		{
			if (!(bmap.mask[byte] & (0x1 << bit)))
			{
				bmap.mask[byte] = bmap.mask[byte] | (0x1 << bit);
				memset(buffer, 0, blocksize);
				writesect(buffer, 8 * byte + bit);
				return 8 * byte + bit;
			}
		}
	}
	panic("Disk is full!\n");
	return 0;
}
void append_a_block(int fd)
{
	int inode_offset = fcb[fd].inode_offset;
	int block_num = (fcb[fd].file_size / blocksize) + !!(fcb[fd].file_size % blocksize);
	block_num -= 1;
	while (block_num >= (int)(blocksize / sizeof(uint32_t) - 1))
	{
		block_num -= (blocksize / sizeof(uint32_t) - 1);
		inode_offset = Inode[inode_offset].data_block_offsets[blocksize / sizeof(uint32_t) - 1];
	}
	block_num += 1;
	if (block_num == blocksize / sizeof(uint32_t) - 1)
	{
		int j = request_free_inode();
		Inode[inode_offset].data_block_offsets[blocksize / sizeof(uint32_t) - 1] = j;
		inode_offset = j;
		Inode[inode_offset].data_block_offsets[0] = request_free_data_block();
	}
	else
	{
		Inode[inode_offset].data_block_offsets[block_num] = request_free_data_block();
	}
}

/* Return the number of bytes read. If it is not for read, return -1. */
int read(int fd, void *buf, int len)
{
	if (fd < 0 || fd >= FCBNUM) return -1;
	if (fcb[fd].state != FS_READ && fcb[fd].state != FS_WR) return -1;
	if (len <= 0) return 0;	//for safe and sound
	unsigned char *start = (unsigned char *)buf;
	int inode_offset = fcb[fd].inode_offset;
	int offset = fcb[fd].offset;
	int count = (len > fcb[fd].file_size - fcb[fd].offset) ? fcb[fd].file_size - fcb[fd].offset : len;
	read_a_part_of_file(start, inode_offset, offset, count);
	fcb[fd].offset += count;
	return count;
}

int write(int fd, void *buf, int len)
{
	if (fd < 0 || fd >= FCBNUM) return -1;
	if (fcb[fd].state != FS_WRITE && fcb[fd].state != FS_WR) return -1;
	if (len <= 0) return 0;	//for safe and sound
	unsigned char *start = (unsigned char *)buf;
	int inode_offset = fcb[fd].inode_offset;
	int offset = fcb[fd].offset;
	int count = len;
	
	int block_num = (fcb[fd].file_size / blocksize) + !!(fcb[fd].file_size % blocksize);
	block_num -= 1;
	
	int end_block = (offset + count - 1) / blocksize;
	int i;
	for (i = 0; i < end_block - block_num; i++) append_a_block(fd);
	
	write_a_part_of_file(start, inode_offset, offset, count);
	
	if (offset + count > fcb[fd].file_size)
	{
		fcb[fd].file_size = offset + count;
		fcb[fd].direntry->file_size = offset + count;
	}
	fcb[fd].offset += count;
	fs_writeback();
	return count;
}

/* Return -1 if an error occurs, and move distance otherwise. */
int lseek(int fd, int offset, enum FS_WHENCE whence)
{
	int origin = fcb[fd].offset;
	if (fd < 0 || fd >= FCBNUM) return -1;
	if (fcb[fd].state != FS_READ && fcb[fd].state != FS_WRITE && fcb[fd].state != FS_WR) return -1;
	switch (whence)
	{
		case SEEK_SET:
			fcb[fd].offset = 0;
			break;
		case SEEK_END:
			fcb[fd].offset = fcb[fd].file_size;
			break;
		case SEEK_CUR:
			break;
		default: panic("Should not get here.\n");
	}
	if (fcb[fd].offset + offset < 0) fcb[fd].offset = 0;
	else if (fcb[fd].offset + offset > fcb[fd].file_size) fcb[fd].offset = fcb[fd].file_size;
	else fcb[fd].offset += offset;
	return fcb[fd].offset - origin;
}

/* Return 0 is successful. Otherwise return -1. */
int close(int fd)
{
	if (fd < 0 || fd >= FCBNUM) return -1;
	fcb[fd].state = FS_CLOSE;
	fcb[fd].offset = 0;
	fcb[fd].direntry = NULL;
	fcb[fd].inode_offset = 0;
	fcb[fd].file_size = 0;
	fcb_free(fd);
	return 0;
}

void read_file(unsigned char *start, int count, int offset)
{
	if (root.entries[2].file_size == 0) panic("The loaded file is empty!\n");
	int inode_offset = root.entries[2].inode_offset;
	if (count <= 0) return;	//for safe and sound
	read_a_part_of_file(start, inode_offset, offset, count);
}
