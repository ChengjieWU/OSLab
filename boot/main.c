#include "boot.h"

/* Now we have extended boot. Boot will load extended boot first. These will never be used.*/
//#define KOFFSET 0xC0000000			/* This is not the only definition. in kernel/include/memory.h*/
//#define ELF_OFFSET_IN_DISK (512 * 127)/* This is not the only definition. */

void readseg(unsigned char *, int, int);

void bootmain(void) {
	
	struct ELFHeader *elf;
	struct ProgramHeader *ph, *eph;
	unsigned char* pa, *i;

	/* 因为引导扇区只有512字节，我们设置了堆栈从0x8000向下生长。*/
	/* Here it doesn't mean stack. It means where to store this local variable elf. */
	elf = (struct ELFHeader*)0x8000;

	readseg((unsigned char*)elf, 4096, 0);

	ph = (struct ProgramHeader*)((char *)elf + elf->phoff);
	eph = ph + elf->phnum;
	for(; ph < eph; ph++)
	{
		pa = (unsigned char*)ph->paddr;
		readseg(pa, ph->filesz, ph->off);
		for (i = pa + ph->filesz; i < pa + ph->memsz; *i++ = 0);
	}
	((void(*)(void))elf->entry)();
}
