#include <stdio.h>
#include <string.h>
#include "fs.h"

unsigned char buffer[blocksize];
struct dir root;
struct inode Inode[FILENUM];
struct bitmap bmap;

int request_free_root_dirent()
{
	int i;
	for (i = 0; i < blocksize / sizeof(struct dirent); i++)
	{
		if (root.entries[i].file_size == 0) break;
	}
	if (i == blocksize / sizeof(struct dirent)) printf("ERROR!\n");
	return i;
}
int request_free_inode()
{
	int j;
	for (j = 0; j < FILENUM; j++)
	{
		if (Inode[j].data_block_offsets[0] == 0) break;
	}
	if (j == FILENUM) printf("ERROR!\n");
	return j;
}


void copy_file(const char *filename)
{
	FILE *disk = fopen("disk.bin", "rb+");
	FILE *kernel = fopen(filename, "rb");
	
	fseek(kernel, 0, SEEK_END);
	unsigned kernel_size = ftell(kernel);
	fseek(kernel, 0, SEEK_SET);
	//int kernel_block = (kernel_size / blocksize) + 1;
	int kernel_block = kernel_size / blocksize + !!(kernel_size % blocksize);
	
	fseek(disk, ROOTOFFSET * blocksize, SEEK_SET);
	fread((unsigned char *)&root, blocksize, 1, disk);
	fseek(disk, INODEOFFSET * blocksize, SEEK_SET);
	fread((unsigned char *)Inode, blocksize * FILENUM, 1, disk);
	fseek(disk, BMAPOFFSET * blocksize, SEEK_SET);
	fread((unsigned char *)&bmap, blocksize * BMAPBLOCK, 1, disk);
	
	
	int i = request_free_root_dirent();

	root.entries[i].file_size = kernel_size;
	strcpy(root.entries[i].filename, filename);
	
	int j = request_free_inode();

	root.entries[i].inode_offset = j;
	
	//printf("%d %d %d\n", i, j, kernel_block);
	
	int count = 0;
	int inode_count = 0;
	int byte;
	for (byte = 0; byte < BMAPTERM; byte++)
	{
		int bit;
		for (bit = 0; bit < 8; bit++)
		{
			if (!(bmap.mask[byte] & (0x1 << bit)))
			{
				bmap.mask[byte] = bmap.mask[byte] | (0x1 << bit);
				int block_num = 8 * byte + bit;
				fread(buffer, blocksize, 1, kernel);
				count++;
				//printf("%d %d\n", block_num, count);
				fseek(disk, block_num * blocksize, SEEK_SET);
				fwrite(buffer, blocksize, 1, disk);
				Inode[j].data_block_offsets[inode_count++] = block_num;
				if (inode_count == blocksize / sizeof(unsigned) - 1)
				{
					int tmp = request_free_inode();
					Inode[j].data_block_offsets[inode_count] = tmp;	//it stores the inode index of the extended one
					j = tmp;
					inode_count = 0;
				}
				if (count == kernel_block) break;
			}
		}
		if (count == kernel_block) break;
	}
	
	//if (count == kernel_block) printf("Yes!\n");
	
	fseek(disk, ROOTOFFSET * blocksize, SEEK_SET);
	fwrite((unsigned char *)&root, blocksize, 1, disk);
	fseek(disk, INODEOFFSET * blocksize, SEEK_SET);
	fwrite((unsigned char *)Inode, blocksize * FILENUM, 1, disk);
	fseek(disk, BMAPOFFSET * blocksize, SEEK_SET);
	fwrite((unsigned char *)&bmap, blocksize * BMAPBLOCK, 1, disk);
	
	fclose(kernel);
	fclose(disk);
	
	printf("Copy %s to disk\n", filename);
}


int main()
{
	copy_file("kernel.bin\0");
	copy_file("sem.bin\0");
	copy_file("game.bin\0");
	copy_file("test.bin\0");
	copy_file("universe.dat\0");
	copy_file("success.dat\0");
	copy_file("failure.dat\0");
	return 0;
}
