#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

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

int fopen(const char *pathname, enum FS_STATE state);
int fclose(int fd);
int flseek(int fd, int offset, enum FS_WHENCE whence);
int fread(int fd, void *buf, int len);
int fwrite(int fd, void *buf, int len);


#endif
