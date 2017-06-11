#ifndef __FS_H__
#define __FS_H__


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
