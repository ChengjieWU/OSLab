#include "sys/syscall.h"
#include "types.h"

#include "wthread.h"

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

int sleep(int t) {
	return syscall(SYS_wait4, t);
}

int drop_exec() {
	return syscall(SYS_drop_exec);
}

semaphore *sem_open(int t, int value) {
	return (semaphore *)syscall(SYS_sem_open, t, value);
}

int sem_close(semaphore *sem) {
	return syscall(SYS_sem_close, sem);
}

int sem_wait(semaphore *sem) {
	return syscall(SYS_sem_wait, sem);
}

int sem_post(semaphore *sem) {
	return syscall(SYS_sem_post, sem);
}

int sem_init(semaphore *sem, int value) {
	return syscall(SYS_sem_init, sem, value);
}

int sem_destroy(semaphore *sem) {
	return syscall(SYS_sem_destroy, sem);
}

int wthread_create(wthread *thread, void *func, void *arg) {
	return syscall(SYS_wthread_create, func, arg, thread);
}

int wthread_exit() {
	return syscall(SYS_wthread_exit);
}

int fwrite(int fd, void *buf, int len) {
	return syscall(SYS_write, fd, buf, len);
}

int fopen(const char *pathname, int state) {
	return syscall(SYS_open, pathname, state);
}

int fclose(int fd) {
	return syscall(SYS_close, fd);
}

int flseek(int fd, int offset, int whence) {
	return syscall(SYS_lseek, fd, offset, whence);
}

int fread(int fd, void *buf, int len) {
	return syscall(SYS_read, fd, buf, len);
}

int ls(int a, int l, int h) {
	return syscall(SYS_ls, a, l, h);
}
