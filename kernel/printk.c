#include "common.h"
#include "stdio.h"

void serial_printc(char);

int __attribute__((__noinline__))
printk(const char *ctl, ...) {
	static char buf[256];
	void *args = (void **)&ctl + 1;
	int len = vsnprintf(buf, 256, ctl, args);
	int i;
	for(i = 0; i < len; ++ i) {
		serial_printc(buf[i]);
	}
	return len;
}
