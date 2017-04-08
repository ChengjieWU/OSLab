#include "common.h"
#include "device/video.h"
#include "x86.h"
#include "elf.h"
#include "string.h"
#include "device/keyboard.h"
#include "device/timer.h"
#include "process.h"

#include "memory.h"

/* Kernel stack starts at 0x8000000 (end of phy-address), which is set in boot/start.S */
#define USER_STACK 0x6000000 //(96MB)

#define GAME_OFFSET_IN_DISK (10 * 1024 * 1024)

void readseg(unsigned char*,int,int);

extern const unsigned char gImage_Universe[1440000];

#ifdef IA32_PAGE
void init_page();
#endif
void init_segment();
void init_idt();
void init_i8259();
void init_serial();
void add_irq_handle(int,void (*)(void));

void init_cond();
void here_we_go();
void printk_test();

int main()
{
#ifdef IA32_PAGE
	init_page();
	/* After paging is enabled, transform %esp to virtual address. */
	//asm volatile("addl %0, %%esp" : : "i"(KOFFSET));
	asm volatile (" addl %0, %%esp\n\t\
					jmp *%1": : "r"(KOFFSET), "r"(init_cond));
					
	assert(0);
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
	
	/* Printk test */
	printk_test();
	
	/* Create and test video memory write and read */
	init_vmem_addr();
	init_vmem_space();
	init_vmem();
	vmem_writing_test();
	vmem_reading_test();
	init_vmem();
	
	printk("Here we go!\n");
	
	
	//((void(*)(void))elf->e_entry)(); /* Here we go! *//* Old jumper, will never use. */
	//here_we_go();

	panic("should not get here!");
}

void here_we_go()
{
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
	
	/* set tss.esp0 to current kernel stack	position, where trap frame will be built*/
	sti();
	asm volatile("movl %%esp, %0" : "=r"(tss.esp0));
	
	asm volatile("movl %0, %%eax" : : "r"(elf->e_entry));
	
	asm volatile("pushl %0" : : "i"(SELECTOR_USER(4)));	//change to user's segments
	asm volatile("popl %ds");
	asm volatile("pushl %0" : : "i"(SELECTOR_USER(4)));
	asm volatile("popl %es");
	asm volatile("pushl %0" : : "i"(SELECTOR_USER(4)));
	asm volatile("popl %fs");
	asm volatile("pushl %0" : : "i"(SELECTOR_USER(4)));
	asm volatile("popl %gs");
	
	asm volatile("pushl %0" : : "i"(SELECTOR_USER(4)));	//push user's ss
	asm volatile("pushl %0" : : "i"(USER_STACK));		//push user's esp
	asm volatile("pushfl");								//push eflags
	asm volatile("pushl %0" : : "i"(SELECTOR_USER(3)));	//push user's cs
	asm volatile("pushl %eax");							//push user's eip
	
	/* Here we go! */
	asm volatile("iret");
}

void printk_test()
{
	printk("\n");
	printk("Printk test begin...\n");
	printk("the answer should be:\n");
	printk("#######################################################\n");
	printk("Hello, welcome to OSlab! I'm the body of the game. ");
	printk("Bootblock loads me to the memory position of 0x100000, and Makefile also tells me that I'm at the location of 0x100000. ");
	printk("~!@#$^&*()_+`1234567890-=...... ");
	printk("Now I will test your printk: ");
	printk("1 + 1 = 2, 123 * 456 = 56088\n0, -1, -2147483648, -1412505855, -32768, 102030\n0, ffffffff, 80000000, abcdef01, ffff8000, 18e8e\n");
	printk("#######################################################\n");
	printk("your answer:\n");
	printk("=======================================================\n");
	printk("%s %s%scome %co%s", "Hello,", "", "wel", 't', " ");
	printk("%c%c%c%c%c! ", 'O', 'S', 'l', 'a', 'b');
	printk("I'm the %s of %s. %s 0x%x, %s 0x%x. ", "body", "the game", "Bootblock loads me to the memory position of",
    0x100000, "and Makefile also tells me that I'm at the location of", 0x100000);
	printk("~!@#$^&*()_+`1234567890-=...... ");
	printk("Now I will test your printk: ");
	printk("%d + %d = %d, %d * %d = %d\n", 1, 1, 1 + 1, 123, 456, 123 * 456);
	printk("%d, %d, %d, %d, %d, %d\n", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);
	printk("%x, %x, %x, %x, %x, %x\n", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);
	printk("=======================================================\n");
	printk("Test end!!! Good luck!!!\n");
	printk("\n");
}
