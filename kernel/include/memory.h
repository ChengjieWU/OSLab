#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "x86.h"


#ifdef IA32_PAGE

#define KOFFSET 0xC0000000			/* This is not the only definition. in boot/main.c */
#define USER_STACK_TOP 0xB0000000

CR3* get_kcr3();
PDE* get_kpdir();
PTE* get_kptable();

void make_invalid_pde(PDE *);
void make_invalid_pte(PTE *);
void make_pde(PDE *, void *);
void make_pte(PTE *, void *);
void make_pde_kernel(PDE *, void *);
void make_pte_kernel(PTE *, void *);
void make_pde_mask(PDE *, void *, uint32_t);
void make_pte_mask(PTE *, void *, uint32_t);
uintptr_t va_pte(PDE* p);
uintptr_t va_byte(PTE *p);

uint32_t request_for_page();
void page_remove(PDE *, void *);

#define va_to_pa(addr) ((void*)(((uint32_t)(addr)) - KOFFSET))

#define pa_to_va(addr) ((void*)(((uint32_t)(addr)) + KOFFSET))
	
#endif


/* the maxinum kernel size is 16MB */
#define KMEM				(16 * 1024 * 1024)

/* 128MB physical memory  */
#define PHY_MEM				(128 * 1024 * 1024)

#ifdef IA32_PAGE

#define NR_PAGE				(PHY_MEM / PAGE_SIZE)
#define NR_KERNEL_PAGE		(KMEM / PAGE_SIZE)
#define NR_USER_PAGE		(NR_PAGE - NR_KERNEL_PAGE)
#define USER_PAGE_OFFSET	NR_KERNEL_PAGE

#endif


#endif
