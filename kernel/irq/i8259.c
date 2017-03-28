#include "x86.h"

#define PORT_PIC_MASTER 0x20
#define PORT_PIC_SLAVE  0xA0
#define IRQ_OFFSET		0x20
#define IRQ_SLAVE       2

void init_i8259(void) {
	/* mask all interrupts */
	out_byte(PORT_PIC_MASTER + 1, 0xff);
	out_byte(PORT_PIC_SLAVE + 1 , 0xff);
	
	/* start initialization */
	out_byte(PORT_PIC_MASTER, 0x11);
	out_byte(PORT_PIC_MASTER + 1, IRQ_OFFSET);
	out_byte(PORT_PIC_MASTER + 1, 1 << IRQ_SLAVE);
	out_byte(PORT_PIC_MASTER + 1, 0x3); // Modified part
	out_byte(PORT_PIC_SLAVE, 0x11);
	out_byte(PORT_PIC_SLAVE + 1, IRQ_OFFSET + 8);
	out_byte(PORT_PIC_SLAVE + 1, IRQ_SLAVE);
	out_byte(PORT_PIC_SLAVE + 1, 0x3); // Modified part
	out_byte(PORT_PIC_MASTER, 0x68);
	out_byte(PORT_PIC_MASTER, 0x0A);
	out_byte(PORT_PIC_SLAVE, 0x68);
	out_byte(PORT_PIC_SLAVE, 0x0A);
}
