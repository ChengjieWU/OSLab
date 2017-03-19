#include "device/video_mode.h"
#include "picture.h"
#include "string.h"
#include "device/video.h"

uint8_t *vmem;

void init_vmem_addr() {
	struct ModeInfoBlock *MIB = (struct ModeInfoBlock *)(0x7e00);
	vmem = (uint8_t *)MIB->physbase;
}

void init_vmem() {
	memcpy(vmem, gImage_Universe, SCR_SIZE);
}

void load_vmem(const uint8_t *buffer, int position, int size) {
	if (position >= SCR_SIZE) assert(0);
	memcpy(vmem + position, buffer, size);
}
