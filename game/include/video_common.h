#ifndef __VIDEO_COMMON_H__
#define __VIDEO_COMMON_H__


#define SCR_WIDTH		800
#define SCR_HEIGHT		600
#define SCR_DEPTH		3
#define SCR_WIDTH_SIZE	(SCR_WIDTH * SCR_DEPTH)
#define SCR_SIZE		((SCR_WIDTH) * (SCR_HEIGHT) * (SCR_DEPTH))
//#define VMEM_ADDR		0xFD000000
//uint32_t VMEM_ADDR;

union Pixels {
	struct {
		uint8_t blue;
		uint8_t green;
		uint8_t red;
	};
	uint8_t RGB_array[3];
	uint32_t RGB_value : 24;
};



#endif
