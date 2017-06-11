#include "common.h"
#include "device/video.h"
#include "x86.h"
#include "elf.h"
#include "string.h"

#include "process.h"

#include "memory.h"

/* Kernel stack starts at 0xc1000000 (NOT end of phy-address), which is set in boot/start.S */
#define USER_STACK KOFFSET

/******************            ************************/
#define ELF_OFFSET_IN_DISK (512 * 8)
#define GAME_OFFSET_IN_DISK KMEM + ELF_OFFSET_IN_DISK

void readseg(unsigned char*,int,int);

#ifdef IA32_PAGE
void init_page();
void init_mm();
PDE* init_updir();
#endif

void init_segment();
void init_idt();
void init_i8259();
void init_serial();
void init_timer();

void add_irq_handle(int,void (*)(void));
void keyboard_event();
void timer_event();

void init_PCB();
void change_to_process(PCB*);
PCB* new_process();
void load_process_memory(PCB *pcb);
PCB* pop_ready_list();
void init_Sem();

void init();
void first_loader();
void printk_test();



void init_fs();




int main()
{
#ifdef IA32_PAGE
	init_page();
	/* After paging is enabled, transform %esp to virtual address. */
	//asm volatile("addl %0, %%esp" : : "i"(KOFFSET));
	asm volatile (" addl %0, %%esp\n\t\
					jmp *%1": : "r"(KOFFSET), "r"(init));	
	panic("should not reach here");
#else
	/* These will never be used. */
	asm volatile("jmp *%0" : : "r"(init));

	panic("should not reach here");
#endif
	return 0;
}

void init()
{
	init_serial();
	init_segment();
	init_idt();
	init_i8259();
	add_irq_handle(0, timer_event);
	add_irq_handle(1, keyboard_event);
	init_timer();
	
	printk("\nBase elements initialized.\n");
	/* Printk test */
	//printk_test();
	
	/* Create and test video memory write and read */
	printk("Initialize video...\n");
	init_vmem_addr();
#ifdef IA32_PAGE
	init_vmem_space();
	init_vmem();
	vmem_writing_test();
	vmem_reading_test();
#endif
	init_vmem();
	
	printk("Initialize memory management...\n");
	init_mm();
	
	printk("Initialize process control block...\n");
	init_PCB();
	printk("Initialize semaphores...\n");
	init_Sem();
	//((void(*)(void))elf->e_entry)(); /* Here we go! *//* Old jumper, will never use. */
	
	init_fs();
	
	first_loader();

	panic("should not get here!");
}

void first_loader()
{
	printk("Creating the frist process...\n");
	PCB *pro = new_process();
	pro = pop_ready_list();
	load_process_memory(pro);
	printk("haha\n");
	change_to_process(pro);

	printk("Loading user programme...\n");
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
	
	printk("Here we go!\n\n");
	/* Now we have PCB! Kernel stack is stored in PCB, and allocated by mm, This will never use! */
	/* set tss.esp0 to current kernel stack	position, where trap frame will be built*/
	//asm volatile("movl %%esp, %0" : "=r"(tss.esp0));
	
	//uint32_t eflags_t;
	//asm volatile("movl %%eflags, %0" : "=r"(eflags_t));
	//eflags_t = eflags_t | 0x200;
	
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
	
	/* This is not strict here. There are still chances to have kernel interruptions. */
	sti();
	asm volatile("pushfl");								//push eflags
	cli();
	
	//asm volatile("push %0" : : "r"(eflags_t));
	
	asm volatile("pushl %0" : : "i"(SELECTOR_USER(3)));	//push user's cs
	asm volatile("pushl %eax");							//push user's eip
	
	current->cpuTime = 0;
	tss.esp0 = (uint32_t)current->kernelStackBottom + PAGE_SIZE;
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
