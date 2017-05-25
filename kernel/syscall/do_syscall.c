#include "common.h"
#include "irq.h"
#include "sys/syscall.h"

#include "semaphore.h"

void serial_printc(char);
void load_vmem(const uint8_t *, int, int);
void fullScreen(const unsigned char*);
volatile uint32_t get_time();
int handle_keys();
void fork_kernel();
int get_pid();
void exit_kernel();
void sleep_kernel(uint32_t);
semaphore *sem_open_kernel(int);
int sem_close_kernel(semaphore *);
int sem_wait_kernel(semaphore *);
int sem_post_kernel(semaphore *);
void wthread_create_kernel(void *, void *);
void wthread_exit_kernel();

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
		//asm volatile (".byte 0xd6" : : "a"(2), "c"(buf), "d"(len));
		int i = 0;
		for (i = 0; i < len; i++) {
			serial_printc(buf[i]);
		}
		tf->eax = len;
	}
	/*else
	{
		tf->eax = fs_write(fd, buf, len);
	}*/
}

/*
static void sys_open(TrapFrame *tf) 
{
	tf->eax = fs_open((const char*)tf->ebx, tf->ecx);
}

static void sys_close(TrapFrame *tf) 
{
	tf->eax = fs_close(tf->ebx);
}

static void sys_read(TrapFrame *tf) 
{
	tf->eax = fs_read(tf->ebx, (char*)tf->ecx, tf->edx);
}

static void sys_lseek(TrapFrame *tf) {
	tf->eax = fs_lseek(tf->ebx, tf->ecx, tf->edx);
}
*/

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
		case SYS_sem_open: tf->eax = (int)sem_open_kernel((int)tf->ebx); break;
		case SYS_sem_close: tf->eax = sem_close_kernel((semaphore *)tf->ebx); break;
		case SYS_sem_wait: tf->eax = sem_wait_kernel((semaphore *)tf->ebx); break;
		case SYS_sem_post: tf->eax = sem_post_kernel((semaphore *)tf->ebx); break;
		case SYS_wthread_create: wthread_create_kernel((void *)tf->ebx, (void *)tf->ecx); break;
		case SYS_wthread_exit: wthread_exit_kernel(); break;
		//case SYS_read: sys_read(tf); break;
		//case SYS_open: sys_open(tf); break;
		//case SYS_lseek: sys_lseek(tf); break;
		//case SYS_close: sys_close(tf); break;

		default: panic("Unhandled system call: id = %d, eip = 0x%08x", tf->eax, tf->eip);
	}
}

