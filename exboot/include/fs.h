#ifndef __FS_H__
#define __FS_H__

/* I have 3 copies of fs.h, they need to be the same. */
/* One in kerel/include, one in tools and the last one in exboot/include. */
/* Any modifications to one of this must be made to others as well. */

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

#pragma pack(0)
struct bitmap{
	unsigned char mask[BMAPTERM];
};
#pragma pack()

#pragma pack(0)
struct dirent {
  char filename[FILEMAXLEN];
  unsigned file_size;
  unsigned inode_offset;
};
#pragma pack()

#pragma pack(0)
struct dir {
  struct dirent entries[blocksize / sizeof(struct dirent)];
};
#pragma pack()

#pragma pack(0)
struct inode {
  unsigned data_block_offsets[blocksize / sizeof(unsigned)];
};
#pragma pack()


#endif
