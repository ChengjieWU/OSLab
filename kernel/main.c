#include "common.h"
#include "device/video.h"
#include "x86.h"
#include "elf.h"
#include "string.h"
#include "device/keyboard.h"
#include "device/timer.h"
#include "process.h"



#define SECTSIZE 512
#define GAME_OFFSET_IN_DISK (10 * 1024 * 1024)
void readseg(unsigned char*,int,int);


#ifdef IA32_PAGE
void init_page();
#endif
void init_segment();
void init_idt();
void init_i8259();
void init_serial();
void add_irq_handle(int,void (*)(void));
void init_cond();


int main()
{
#ifdef IA32_PAGE
	init_page();
	/* After paging is enabled, transform %esp to virtual address. */
	asm volatile("addl %0, %%esp" : : "i"(KOFFSET));
#endif
	/* Jump to init_cond() to continue initialization. */
	asm volatile("jmp *%0" : : "r"(init_cond));

	panic("should not reach here");
	return 0;
}

void init_cond()
{
	init_segment();
	init_idt();
	
	init_i8259();
	init_serial();
	add_irq_handle(0, timer_event);
	add_irq_handle(1, keyboard_event);
	init_timer();
	
	sti();
	init_vmem_addr();
	init_vmem();
	printk("Here we go!\n");
	
	struct Elf *elf;
	struct Proghdr *ph, *eph;
	unsigned char *pa, *i;
	
	uint8_t buf[4096];
	elf = (struct Elf*)buf;

	readseg((unsigned char*)elf, 4096, GAME_OFFSET_IN_DISK);

	ph = (struct Proghdr*)((char *)elf + elf->e_phoff);
	eph = ph + elf->e_phnum;
	for(; ph < eph; ph ++) {
		pa = (unsigned char*)ph->p_pa;
		readseg(pa, ph->p_filesz, GAME_OFFSET_IN_DISK + ph->p_offset);
		for(i = pa + ph->p_filesz; i < pa + ph->p_memsz; *i ++ = 0);
	}
	
	//((void(*)(void))elf->e_entry)(); /* Here we go! */
	
	
	asm volatile("pushl %0" : : "g"(SELECTOR_USER(4)));
	asm volatile("popl %%ds" : : : "memory");
	asm volatile("pushl %0" : : "g"(SELECTOR_USER(4)));
	asm volatile("popl %%es" : : : "memory");
	asm volatile("pushl %0" : : "g"(SELECTOR_USER(4)));
	asm volatile("popl %%fs" : : : "memory");
	asm volatile("pushl %0" : : "g"(SELECTOR_USER(4)));
	asm volatile("popl %%gs" : : : "memory");
	asm volatile("pushl %0" : : "g"(SELECTOR_USER(4)));
	asm volatile("pushl %0" : : "g"(0x700000));
	asm volatile("pushfl");
	asm volatile("pushl %0" : : "g"(SELECTOR_USER(3)));
	asm volatile("pushl %0" : : "g"(elf->e_entry));
	asm volatile("iret");
	
	
	init_vmem();
	
	assert(0);
}


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

	//insl(0x1f0, dst, SECTSIZE/4);	//read a sector
	//	this part does the same thing
	int i;
	for(i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = in_long(0x1f0);
	}
}

void readseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 1;
	for(; pa < epa; pa += SECTSIZE, offset ++)
		readsect(pa, offset);
}
