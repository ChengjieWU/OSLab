#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "common.h"

#define SCR_WIDTH		800
#define SCR_HEIGHT		600
#define SCR_DEPTH		3
#define SCR_WIDTH_SIZE	(SCR_WIDTH * SCR_DEPTH)
#define SCR_SIZE		((SCR_WIDTH) * (SCR_HEIGHT) * (SCR_DEPTH))

#define VMEM_ADDR		0xFD000000		/* The value is got, not set. */


union Pixels {
	struct {
		uint8_t blue;
		uint8_t green;
		uint8_t red;
	};
	uint8_t RGB_array[3];
	uint32_t RGB_value : 24;
};

struct Surface {
	int x, y;
	int w, h;
	uint8_t *pixels;
};

extern uint8_t *vmem;

void init_vmem_addr();
void init_vmem();
void init_vmem_space();
void vmem_writing_test();
void vmem_reading_test();
void load_vmem(const uint8_t*, int, int);
void fullScreen(const unsigned char*);

#endif
