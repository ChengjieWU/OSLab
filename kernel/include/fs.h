#ifndef __FS_H__
#define __FS_H__

#include "common.h"

#define DISKBLOCK 20000
#define blocksize 512
#define FILEMAXLEN 24

#define BMAPOFFSET 1
#define BMAPBLOCK (DISKBLOCK / (blocksize * 8) + 1)
#define BMAPTERM (BMAPBLOCK * 512)

#define INODEOFFSET (BMAPOFFSET + BMAPBLOCK)
#define FILENUM 120

#define ROOTOFFSET (INODEOFFSET + FILENUM)

#define DATAOFFSET (ROOTOFFSET + 1)

#pragma pack(0)
struct bitmap{
	uint8_t mask[BMAPTERM];
};
#pragma pack()

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

/* The followings are not in formatter tools. */

#define FCBNUM 20

enum FS_STATE {
    FS_WRITE,
    FS_READ,
    FS_WR,
    FS_CLOSE
};

enum FS_WHENCE {
	SEEK_SET,
	SEEK_CUR,
	SEEK_END
};

struct FCB {
	int index;
	uint32_t inode_offset;
	uint32_t file_size;
	uint32_t offset;
	enum FS_STATE state;
	struct FCB *next;
	struct dirent *direntry;
};


#endif
