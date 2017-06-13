#include "fs.h"
#include "string.h"
#include "disk.h"

extern unsigned char buffer[blocksize];
extern struct bitmap bmap;
extern struct dir root;
extern struct inode Inode[FILENUM];
extern struct FCB fcb[FCBNUM];
extern struct FCB *fcb_free_list;


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

