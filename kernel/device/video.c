#include "device/video_mode.h"
#include "string.h"
#include "device/video.h"
#include "x86.h"

uint8_t *vmem;

void init_vmem_addr() {
	struct ModeInfoBlock *MIB = (struct ModeInfoBlock *)(0x7e00);
	vmem = (uint8_t *)MIB->physbase;
	/* Note: MIB->physbase = fd000000 */
}

void init_vmem() {
	memset(vmem, 0xff, SCR_SIZE);
}

void load_vmem(const uint8_t *buffer, int position, int size) {
	if (position >= SCR_SIZE) assert(0);
	memcpy(vmem + position, buffer, size);
}

void fullScreen(const unsigned char* src)
{
	memcpy(vmem, src, SCR_SIZE);
}
