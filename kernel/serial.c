#include "x86.h"

#define PORT 0x3f8   /* COM1 */


void init_serial() {
	out_byte(PORT + 1, 0x00);
	out_byte(PORT + 3, 0x80);
	out_byte(PORT + 0, 0x03);
	out_byte(PORT + 1, 0x00);
	out_byte(PORT + 3, 0x03);
	out_byte(PORT + 2, 0xC7);
	out_byte(PORT + 4, 0x0B);
}

int is_serial_idle() {
	return in_byte(PORT + 5) & 0x20;
}

void serial_printc(char a){
	while(!is_serial_idle());
	out_byte(PORT, a);
}
