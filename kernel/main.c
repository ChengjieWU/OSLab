#include "common.h"
#include "device/video.h"
#include "device/video_mode.h"
#include "x86.h"

void init_idt();
void init_i8259();
void init_serial();
void init_timer();
void add_irq_handle(int,void (*)(void));
void timer_event();
void keyboard_event();



int main()
{
	init_idt();
	
	init_i8259();
	init_serial();
	add_irq_handle(0, timer_event);
	add_irq_handle(1, keyboard_event);
	
	sti();
	
	init_vmem_addr();
	init_vmem();
	
	printk("Here we go!\n");
	while(1);
	return 0;
}
