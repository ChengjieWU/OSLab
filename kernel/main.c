#include "common.h"
#include "device/video.h"
#include "x86.h"
#include "device/keyboard.h"
#include "device/timer.h"


void init_idt();
void init_i8259();
void init_serial();
void add_irq_handle(int,void (*)(void));



void game();


int main()
{
	init_idt();
	
	init_i8259();
	init_serial();
	add_irq_handle(0, timer_event);
	add_irq_handle(1, keyboard_event);
	
	sti();
	
	init_vmem_addr();
	
	printk("Here we go!\n");
	game();
	
	init_vmem();
	
	assert(0);
	return 0;
}
