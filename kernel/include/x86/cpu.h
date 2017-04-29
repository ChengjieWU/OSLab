#ifndef __X86_CPU_H__
#define __X86_CPU_H__

#include "common.h"

/* the Control Register 0 */
typedef union CR0 {
	struct {
		uint32_t protect_enable      : 1;
		uint32_t monitor_coprocessor : 1;
		uint32_t emulation           : 1;
		uint32_t task_switched       : 1;
		uint32_t extension_type      : 1;
		uint32_t numeric_error       : 1;
		uint32_t pad0                : 10;
		uint32_t write_protect       : 1; 
		uint32_t pad1                : 1; 
		uint32_t alignment_mask      : 1;
		uint32_t pad2                : 10;
		uint32_t no_write_through    : 1;
		uint32_t cache_disable       : 1;
		uint32_t paging              : 1;
	};
	uint32_t val;
} CR0;

/* the Control Register 3 (physical address of page directory) */
typedef union CR3 {
	struct {
		uint32_t pad0                : 3;
		uint32_t page_write_through  : 1;
		uint32_t page_cache_disable  : 1;
		uint32_t pad1                : 7;
		uint32_t page_directory_base : 20;
	};
	uint32_t val;
} CR3;

/* read CR0 */
static inline uint32_t
read_cr0() {
	uint32_t val;
	asm volatile("movl %%cr0, %0" : "=r"(val));
	return val;
}

/* write CR0 */
static inline void
write_cr0(CR0 *cr0) {
	asm volatile("movl %0, %%cr0" : : "r"(cr0->val));
}

/* read CR2, notice that CR2 is never written, dealing with page fault! */
static inline uint32_t
read_cr2() {
	uint32_t val;
	asm volatile("movl %%cr2, %0" : "=r"(val));
	return val;
}

/* write CR3, notice that CR3 is never read */
static inline void
write_cr3(CR3 *cr3) {
	asm volatile("movl %0, %%cr3" : : "r"(cr3->val));
}

/* modify the value of GDTR */
static inline void
write_gdtr(void *addr, uint32_t size) {
	static volatile uint16_t data[3];
	data[0] = size - 1;
	data[1] = (uint32_t)addr;
	data[2] = ((uint32_t)addr) >> 16;
	asm volatile("lgdt (%0)" : : "r"(data));
}

/* modify the value of IDTR */
static inline void
write_idtr(void *addr, uint32_t size) {
	static volatile uint16_t data[3];
	data[0] = size - 1;
	data[1] = (uint32_t)addr;
	data[2] = ((uint32_t)addr) >> 16;
	asm volatile("lidt (%0)" : : "r"(data));
}

/* write TR */
static inline void
write_tr(uint16_t selector) {
	asm volatile("ltr %0" : : "r"(selector));
}

/* read EFLAGS */
static inline uint32_t
read_eflags(void) {
	volatile uint32_t val;
	asm volatile("pushf; popl %0" : "=rm"(val): :"memory");
	return val;
}

/* enable interrupt */
static inline void
sti(void) {
	asm volatile("sti");
}

/* disable interrupt */
static inline void
cli(void) {
	asm volatile("cli");
}

/* put the CPU into idle, waiting for the next interrupt */
static inline void
wait_intr() {
	asm volatile("hlt");
}

/* flush tlb */
static inline void invlpg(void *addr)
{
	asm volatile("invlpg (%0)" : : "r" (addr) : "memory");
}

#define NR_IRQ    256

#endif
