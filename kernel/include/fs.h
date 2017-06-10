#ifndef __FS_H__
#define __FS_H__

#include "common.h"

#define DiskBlockNum 10000
#define blocksize 512
#define FILEMAXLEN 24

#define BMAPOFFSET 1

#pragma pack(0)

struct bitmap{
	uint8_t mask[DiskBlockNum / 8];
};

struct dirent {
  char     filename[FILEMAXLEN];
  uint32_t file_size;
  uint32_t inode_offset;
};

struct dir {
  struct dirent entries[blocksize / sizeof(struct dirent)];
};

struct inode {
  uint32_t data_block_offsets[blocksize / sizeof(uint32_t)];
};

#pragma pack()

#endif
