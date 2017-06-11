#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define DISKBLOCK 10000
#define blocksize 512
#define FILEMAXLEN 24

#define BMAPOFFSET 1
#define BMAPBLOCK (DISKBLOCK / (blocksize * 8) + 1)
#define BMAPTERM (BMAPBLOCK * 512)

#define INODEOFFSET (BMAPOFFSET + BMAPBLOCK)
#define FILENUM 8

#define ROOTOFFSET (INODEOFFSET + FILENUM)

#define DATAOFFSET (ROOTOFFSET + 1)

#pragma pack(0)
struct dirent {
  char     filename[FILEMAXLEN];
  uint32_t file_size;
  uint32_t inode_offset;
};
#pragma pack()

#pragma pack(0)
struct dir {
  struct dirent entries[blocksize / sizeof(struct dirent)];
};
#pragma pack()

#pragma pack(0)
struct inode {
  uint32_t data_block_offsets[blocksize / sizeof(uint32_t)];
};
#pragma pack()

#pragma pack(0)
struct bitmap{
	unsigned char mask[512 * 3];
};
#pragma pack()

struct bitmap bmap;
struct dir root;
struct inode Inode[FILENUM];

unsigned char tmp[512];


int main()
{
	srand((unsigned)time(NULL));
	int i;
	for (i = 0; i < 1250; i++)
	{
		bmap.mask[i] = rand() % 256;
	}
	FILE *fp = fopen("bitmap", "wb");
	fwrite(bmap.mask, 1, 512 * 3, fp);
	
	for (i = 0; i < blocksize / sizeof(uint32_t); i++)
	{
		inode[0].data_block_offsets[i] = DATAOFFSET + i;
	}
	
	root.entries[0].filename = "kernel\0";
	root.entries[0].file_size = 0;
	root.entries[0].inode_offset = 0;

	
	
	fclose(fp);
}
