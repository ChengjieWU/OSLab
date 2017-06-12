#include "x86.h"


#define SECTSIZE 512

void waitdisk(void) {
	while((in_byte(0x1f7) & 0xc0) != 0x40);
}

void readsect(void *dst, int offset) {
	waitdisk();

	out_byte(0x1f2, 1);		// count = 1
	out_byte(0x1f3, offset);
	out_byte(0x1f4, offset >> 8);
	out_byte(0x1f5, offset >> 16);
	out_byte(0x1f6, (offset >> 24) | 0xe0);
	out_byte(0x1f7, 0x20);	// cmd 0x20 - read sectors

	waitdisk();

	int i;
	for(i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = in_long(0x1f0);
	}
}
void writesect(void *dst, int offset) {
	
	if (offset == 0) panic("boot block error!\n");
	waitdisk();
	out_byte(0x1f2, 1);
	out_byte(0x1f3, offset);
	out_byte(0x1f4, offset >> 8);
	out_byte(0x1f5, offset >> 16);
	out_byte(0x1f6, (offset >> 24) | 0xe0);
	out_byte(0x1f7, 0x30);
	waitdisk();
	int i;
	for (i = 0; i < SECTSIZE / 4; i++) {
		out_long(0x1f0, ((int *)dst)[i]);
	}
}

void readsect_n(void *dst, int offset, int num) {
	unsigned char *pa = dst;
	unsigned char *epa = dst + num * SECTSIZE;
	for (; pa < epa; pa += SECTSIZE, offset++) {
		readsect(pa, offset);
	}
}

void writesect_n(void *dst, int offset, int num) {
	unsigned char *pa = dst;
	unsigned char *epa = dst + num * SECTSIZE;
	for (; pa < epa; pa += SECTSIZE, offset++) {
		writesect(pa, offset);
	}
}

/* Boot block is already taken into account. */
void readseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 1;
	for(; pa < epa; pa += SECTSIZE, offset ++)
		readsect(pa, offset);
}
