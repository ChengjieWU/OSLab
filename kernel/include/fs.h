#ifndef __FS_H__
#define __FS_H__

/* I have 3 copies of fs.h, they need to be the same. */
/* One in kerel/include, one in tools and the last one in exboot/include. */
/* Any modifications to one of this must be made to others as well. */
/* This copy is the most important one, it has some unique content, while the others don't. */

#include "common.h"

#define DISKBLOCK 20000
#define blocksize 512
#define FILEMAXLEN 24

/* This is the spaces reserved for exboot. So, it is its maximal size. */
#define EXBOOT 49

#define BMAPOFFSET (1 + EXBOOT)
#define BMAPBLOCK (DISKBLOCK / (blocksize * 8) + 1)
#define BMAPTERM (BMAPBLOCK * 512)

#define INODEOFFSET (BMAPOFFSET + BMAPBLOCK)
#define FILENUM 200

#define ROOTOFFSET (INODEOFFSET + FILENUM)

#define DATAOFFSET (ROOTOFFSET + 1)

/* Assume that the following sturctures occupy 1 blocksize in total. */

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

/* The followings are not in other versions. */

#define FCBNUM 50

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
