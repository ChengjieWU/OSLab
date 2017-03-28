#include "common.h"
#include "device/video.h"
#include "x86.h"
#include "elf.h"
#include "string.h"
#include "device/keyboard.h"
#include "device/timer.h"
#include "process.h"

/* Kernel stack starts at 0x8000000 (end of phy-address), which is set in boot/start.S */
#define USER_STACK 0x6000000 //(96MB)

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
	
	init_vmem_addr();
	init_vmem();
	sti();
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
	
	//((void(*)(void))elf->e_entry)(); /* Here we go! *//* Old jumper, will never use. */
	
	asm volatile("pushl %0" : : "r"(SELECTOR_USER(4)));
	asm volatile("popl %%ds" : : : "memory");
	asm volatile("pushl %0" : : "r"(SELECTOR_USER(4)));
	asm volatile("popl %%es" : : : "memory");
	asm volatile("pushl %0" : : "r"(SELECTOR_USER(4)));
	asm volatile("popl %%fs" : : : "memory");
	asm volatile("pushl %0" : : "r"(SELECTOR_USER(4)));
	asm volatile("popl %%gs" : : : "memory");
	asm volatile("pushl %0" : : "r"(SELECTOR_USER(4)));	//push user's ss
	asm volatile("pushl %0" : : "i"(USER_STACK));		//push user's esp
	asm volatile("pushfl");								//push eflags
	asm volatile("pushl %0" : : "r"(SELECTOR_USER(3)));	//push user's cs
	asm volatile("pushl %0" : : "r"(elf->e_entry));		//push user's eip
	/* set tss.esp0 to current kernel stack	position, where trap frame will be built*/
	asm volatile("movl %%esp, %0" : "=r"(tss.esp0));	
	asm volatile("iret");
	
	init_vmem();
	assert(0);
}
