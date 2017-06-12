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
	for (i = 0; i < FCBNUM; i++)
	{
		fcb[i].index = i;
		fcb[i].state = FS_CLOSE;
		fcb[i].offset = 0;
		fcb[i].next = fcb_free_list;
		fcb_free_list = &fcb[i];
	}
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
	fcb[fd].state = FS_CLOSE;
	fcb[fd].offset = 0;
	fcb[fd].next = fcb_free_list;
	fcb_free_list = &fcb[fd];
	return true;
}

/* Return fd. If cannot find such file, or don't open it, return -1. */
int open(const char *pathname, enum FS_STATE state)
{
	int i;
	for (i = 0; i < blocksize / sizeof(struct dirent); i++)
		if (strcmp(root.entries[i].filename, pathname) == 0) break;
	if (i == blocksize / sizeof(struct dirent) || state == FS_CLOSE) return -1;
	int fd = fcb_allocate();
	if (fd == -1) panic("No free fcb!\n");
	fcb[fd].state = state;
	fcb[fd].offset = 0;
	fcb[fd].inode_offset = root.entries[i].inode_offset;
	fcb[fd].file_size = root.entries[i].file_size;
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

void read_a_part_of_file(unsigned char *start, int inode_offset, int offset, int count)
{
	/* Note that currently, inode_offset is the inode index of the first inode of the file. */
	/* offset is the offset within the whole file, and count is the number of bytes read. */
	int start_bias = offset % blocksize;
	int start_length = blocksize - start_bias;
	int start_block = offset / blocksize;
	int end_block = (offset + count) / blocksize;
	
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

/* Return the number of bytes read. If it is not for read, return -1. */
int read(int fd, void *buf, int len)
{
	if (fd < 0 || fd >= FCBNUM) return -1;
	if (fcb[fd].state != FS_READ && fcb[fd].state != FS_WR) return -1;
	unsigned char *start = (unsigned char *)buf;
	int inode_offset = fcb[fd].inode_offset;
	int offset = fcb[fd].offset;
	int count = (len > fcb[fd].file_size - fcb[fd].offset) ? fcb[fd].file_size - fcb[fd].offset : len;
	read_a_part_of_file(start, inode_offset, offset, count);
	fcb[fd].offset += count;
	return count;
}

/* Return -1 if an error occurs, and 0 otherwise. */
int lseek(int fd, int offset, enum FS_WHENCE whence)
{
	if (fd < 0 || fd >= FCBNUM) return -1;
	if (fcb[fd].state == FS_CLOSE) return -1;
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
	return 0;
}

/* Return 0 is successful. Otherwise return -1. */
int close(int fd)
{
	if (fd < 0 || fd >= FCBNUM) return -1;
	fcb_free(fd);
	return 0;
}

void read_file(unsigned char *start, int count, int offset)
{
	if (root.entries[2].file_size == 0) panic(0);
	int inode_offset = root.entries[2].inode_offset;
	read_a_part_of_file(start, inode_offset, offset, count);
}
