#include "boot.h"


/* In order for kernel to run, KOFFSET must be set. */
#define KOFFSET 0xC0000000

void read_file(unsigned char *start, int count, int offset);
void prepare_fs();

void exbootmain(void)
{
	prepare_fs();
	
	struct ELFHeader *elf;
	struct ProgramHeader *ph, *eph;
	unsigned char* pa, *i;

	/* Same as boot/main.c. */
	elf = (struct ELFHeader*)0x8000;
	
	/* The difference is that we now read using read_file, which is same with that in kernel. */
	read_file((unsigned char*)elf, 4096, 0);

	ph = (struct ProgramHeader*)((char *)elf + elf->phoff);
	eph = ph + elf->phnum;
	for(; ph < eph; ph++)
	{
		pa = (unsigned char*)(ph->paddr - KOFFSET);
		read_file(pa, ph->filesz, ph->off);
		for (i = pa + ph->filesz; i < pa + ph->memsz; *i++ = 0);
	}
	
	((void(*)(void))(elf->entry - KOFFSET))();
}
