#ifndef __DISK_H__
#define __DISK_H__

void readsect(void *dst, int offset);
void readsect_n(void *dst, int offset, int num);
void readseg(unsigned char *pa, int count, int offset);

#endif
