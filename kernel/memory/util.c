#include "x86/memory.h"
#include "memory.h"

void make_invalid_pde(PDE *p) { p->val = 0;}

void make_invalid_pte(PTE *p) { p->val = 0;}

bool is_invalid_pde(PDE *p) { return (p->val == 0); }

bool is_invalid_pte(PTE *p) { return (p->val == 0); }

/* For simplicity, we make all pages readable and writable for all ring 3 processes.
 * In Lab3, you may set different flags for different pages to perform the whole 
 * page level protection. */

void make_pde(PDE *p, void *addr) 
{
	p->val = 0;
	p->page_frame = ((uint32_t)addr) >> 12;
	p->present = 1;
	p->read_write = 1;
	p->user_supervisor = 1;
}

void make_pte(PTE *p, void *addr) 
{
	p->val = 0;
	p->page_frame = ((uint32_t)addr) >> 12;
	p->present = 1;
	p->read_write = 1;
	p->user_supervisor = 1;
}

inline uint32_t get_pte_ind(uint32_t n) {
    return ((n & 0x3ff000)>>12); // 0000 0000 0011 1111 1111 0000 0000 0000
}

/**/
uint32_t va_to_pa_i386(uint32_t pde_addr, uint32_t va) {

    uint32_t tval;
    PDE* pdr = (PDE*)pde_addr; //physical address
    PDE* vdr = (PDE*) pa_to_va(pdr); //virtual address in kernel space
    printk("va_to_pa_i386 vdr= %x\n",vdr);

    tval = (uint32_t)(vdr[va>>22].page_frame);

    PTE* ptable = (PTE*)(tval<<12);
    PTE* vtable = pa_to_va(ptable);
    printk("va_to_pa_i386 vtable= %x\n",vtable);

    tval = (uint32_t)(vtable[(va>>12)& 0x3ff].page_frame); //midel ten
    printk("va_to_pa_i386 tval= %x\n",tval);
    tval = tval<<12;
    printk("va_to_pa_i386 = %x\n",tval);
    return (tval | (va & 0xfff)); // add offset to physical address
}
/*
uint32_t pcb_va_to_pa(PCB *pcb, uint32_t va) {
    uint32_t val = pcb->cr3.page_directory_base;
    val = val<<12;
    printk("pde_addr=%x\n",val);
    return va_to_pa_i386(val,va);

}*/
