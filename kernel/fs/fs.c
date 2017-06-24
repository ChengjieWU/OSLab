#include "fs.h"
#include "string.h"
#include "disk.h"

unsigned char buffer[blocksize];
struct bitmap bmap;
struct dir root;
struct inode Inode[FILENUM];
struct FCB fcb[FCBNUM];
struct FCB *fcb_free_list;

/* Implemented in util.c */
int fcb_allocate();
bool fcb_free(int fd);
int request_free_root_dirent();
uint32_t request_free_inode();
void append_a_block(int fd);

/* Implemented in rw.c */
void read_a_sect_of_file(int block_num, int inode_offset);
void write_a_sect_of_file(int block_num, int inode_offset);
void read_a_part_of_file(unsigned char *start, int inode_offset, int offset, int count);
void write_a_part_of_file(unsigned char *start, int inode_offset, int offset, int count);

/* Implemented in pcbfile.c */
int pcb_fcb_add(int fd);
bool pcb_fcb_test(int fd);
int pcb_fcb_free(int fd);

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
/* Return 0 is successful. Otherwise return -1. */
int close(int fd)
{
	if (fd < 0 || fd >= FCBNUM) return -1;
	if (pcb_fcb_free(fd) == -1) return -1;
	fcb[fd].state = FS_CLOSE;
	fcb[fd].offset = 0;
	fcb[fd].direntry = NULL;
	fcb[fd].inode_offset = 0;
	fcb[fd].file_size = 0;
	fcb_free(fd);
	return 0;
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
	if (pcb_fcb_add(fd) == -1) {fcb_free(fd); printk("A process have full files."); return -1;}
	fcb[fd].state = state;
	fcb[fd].offset = 0;
	fcb[fd].inode_offset = root.entries[i].inode_offset;
	fcb[fd].file_size = root.entries[i].file_size;
	fcb[fd].direntry = &root.entries[i];
	fs_writeback();
	return fd;
}

/* Return the number of bytes read. If it is not for read, return -1. */
int read(int fd, void *buf, int len)
{
	if (fd < 0 || fd >= FCBNUM) return -1;
	if (pcb_fcb_test(fd) == false) return -1;
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
	if (pcb_fcb_test(fd) == false) return -1;
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
	if (pcb_fcb_test(fd) == false) return -1;
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

int find_file(char *name)
{
	int i;
	for (i = 0; i < blocksize / sizeof(struct dirent); i++)
		if (strcmp(root.entries[i].filename, name) == 0) break;
	if (i == blocksize / sizeof(struct dirent)) {return -1;}
	else if (root.entries[i].file_size == 0) {return -1;}
	return i;
}

void load_program(int index, unsigned char *start, int count, int offset)
{
	int i = index;
	int inode_offset = root.entries[i].inode_offset;
	if (count <= 0) return;	//for safe and sound
	read_a_part_of_file(start, inode_offset, offset, count);
}

void read_first_program(unsigned char *start, int count, int offset)
{
	int i;
	for (i = 0; i < blocksize / sizeof(struct dirent); i++)
		if (strcmp(root.entries[i].filename, "myshell.bin\0") == 0) break;
	if (i == blocksize / sizeof(struct dirent)) panic("No game is found!\n");
	else if (root.entries[i].file_size == 0) panic("The loaded file is empty!\n");
	int inode_offset = root.entries[i].inode_offset;
	if (count <= 0) return;	//for safe and sound
	read_a_part_of_file(start, inode_offset, offset, count);
}

void ls_kernel(int a, int l, int h)
{
	int i;
	for (i = 0; i < blocksize / sizeof(struct dirent); i++)
	{
		if (root.entries[i].file_size != 0)
		{
			printk("%s\t", root.entries[i].filename);
			if (h) printk("%d\t", root.entries[i].file_size);
			if (l) printk("%d\t", root.entries[i].inode_offset);
			printk("\n");
		}
	}
}
