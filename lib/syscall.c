#include "sys/syscall.h"
#include "types.h"

static inline int //__attribute__((__noinline__))
syscall(int id, ...) {
	int ret;
	int *args = &id;
	asm volatile("int $0x80": "=a"(ret) : "a"(args[0]), "b"(args[1]), "c"(args[2]), "d"(args[3]));
	return ret;
}


int serial_output(int fd, char *buf, int len) {
	return syscall(SYS_write, fd, buf, len);
}

int loadVideo(const uint8_t *buffer, int position, int size){
	return syscall(SYS_loadVideo, buffer, position, size);
}

int fullVideo(const unsigned char* src) {
	return syscall(SYS_fullVideo, src);
}

uint32_t getTime() {
	return syscall(SYS_time);
}

int readKey() {
	return syscall(SYS_keyboard);
}

int fork() {
	return syscall(SYS_fork);
}

int getpid() {
	return syscall(SYS_getpid);
}

int exit() {
	return syscall(SYS_exit);
}
