#include "common.h"
#include "irq.h"
#include "sys/syscall.h"

#include "wthread.h"

void serial_printc(char);
void load_vmem(const uint8_t *, int, int);
void fullScreen(const unsigned char*);
volatile uint32_t get_time();
int handle_keys();
void fork_kernel();
int get_pid();
void exit_kernel();
void sleep_kernel(uint32_t);
void drop_exec_kernel();
semaphore *sem_open_kernel(int, int);
int sem_close_kernel(semaphore *);
int sem_wait_kernel(semaphore *);
int sem_post_kernel(semaphore *);
int sem_init_kernel(semaphore *, int);
int sem_destroy_kernel(semaphore *);
void wthread_create_kernel(void *, void *, wthread *);
void wthread_exit_kernel();
int open(const char *pathname, int state);
int close(int fd);
int lseek(int fd, int offset, int whence);
int read(int fd, void *buf, int len);
int write(int fd, void *buf, int len);
void ls_kernel(int a, int l, int h);
void exec_kernel(char *name);

/*
uint32_t mm_brk(uint32_t);
int fs_ioctl(int, uint32_t, void *);

int fs_open(const char*, int flags);
int fs_read(int, void*, size_t);
int fs_write(int, const void*, size_t);
off_t fs_lseek(int, off_t, int);
int fs_close(int);


static void sys_brk(TrapFrame *tf) {
	tf->eax = mm_brk(tf->ebx);
}

static void sys_ioctl(TrapFrame *tf) {
	tf->eax = fs_ioctl(tf->ebx, tf->ecx, (void *)tf->edx);
}

*/
static void sys_write(TrapFrame *tf) {
	int fd = tf->ebx;
	char* buf = (char*)tf->ecx;
	int len = tf->edx;
	if (fd == 1 || fd == 2) {
		int i = 0;
		for (i = 0; i < len; i++) {
			serial_printc(buf[i]);
		}
		tf->eax = len;
	}
	else
	{
		tf->eax = write(fd, buf, len);
	}
}

void do_syscall(TrapFrame *tf) {
	switch(tf->eax) {
		/* The `add_irq_handle' system call is artificial. We use it to
		 * let user program register its interrupt handlers. But this is
		 * very dangerous in a real operating system. Therefore such a
		 * system call never exists in GNU/Linux.
		 */

		//case SYS_brk: sys_brk(tf); break;
		//case SYS_ioctl: sys_ioctl(tf); break;

		/* TODO: Add more system calls. */

		case SYS_write: sys_write(tf); break;
		case SYS_loadVideo: load_vmem((const uint8_t *)tf->ebx, tf->ecx, tf->edx); break;
		case SYS_fullVideo: fullScreen((const unsigned char*)tf->ebx); break;
		case SYS_time: tf->eax = get_time(); break;
		case SYS_keyboard: tf->eax = handle_keys(); break;
		case SYS_fork: fork_kernel(); break;
		case SYS_getpid: tf->eax = get_pid(); break;
		case SYS_exit: exit_kernel(); break;
		case SYS_wait4: sleep_kernel((int)tf->ebx); break;
		case SYS_drop_exec: drop_exec_kernel(); break;
		case SYS_sem_open: tf->eax = (int)sem_open_kernel((int)tf->ebx, (int)tf->ecx); break;
		case SYS_sem_close: tf->eax = sem_close_kernel((semaphore *)tf->ebx); break;
		case SYS_sem_wait: tf->eax = sem_wait_kernel((semaphore *)tf->ebx); break;
		case SYS_sem_post: tf->eax = sem_post_kernel((semaphore *)tf->ebx); break;
		case SYS_sem_init: tf->eax = sem_init_kernel((semaphore *)tf->ebx, (int)tf->ecx); break;
		case SYS_sem_destroy: tf->eax = sem_destroy_kernel((semaphore *)tf->ebx); break;
		case SYS_wthread_create: wthread_create_kernel((void *)tf->ebx, (void *)tf->ecx, (wthread *)tf->edx); break;
		case SYS_wthread_exit: wthread_exit_kernel(); break;
		case SYS_read: tf->eax = read(tf->ebx, (char *)tf->ecx, tf->edx); break;
		case SYS_open: tf->eax = open((const char*)tf->ebx, tf->ecx); break;
		case SYS_lseek: tf->eax = lseek(tf->ebx, tf->ecx, tf->edx); break;
		case SYS_close: tf->eax = close(tf->ebx); break;
		case SYS_ls: ls_kernel(tf->ebx, tf->ecx, tf->edx); break;
		case SYS_exec: exec_kernel((char *)tf->ebx); break;
		default: panic("Unhandled system call: id = %d, eip = 0x%08x", tf->eax, tf->eip);
	}
}

