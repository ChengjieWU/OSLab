#include "memory.h"
#include "mmu.h"

void make_invalid_pde(PDE *p) { p->val = 0;}

void make_invalid_pte(PTE *p) { p->val = 0;}

bool is_invalid_pde(PDE *p) { return (p->val == 0); }

bool is_invalid_pte(PTE *p) { return (p->val == 0); }



void make_pde(PDE *p, void *addr) 
{
	make_pde_mask(p, addr, PTE_P | PTE_W | PTE_U);
}

void make_pte(PTE *p, void *addr) 
{
	make_pte_mask(p, addr, PTE_P | PTE_W | PTE_U);
}

void make_pde_kernel(PDE* p, void *addr)
{
	make_pde_mask(p, addr, PTE_P | PTE_W);
}

void make_pte_kernel(PTE* p, void *addr)
{
	make_pte_mask(p, addr, PTE_P | PTE_W);
}

void make_pde_mask(PDE *p, void *addr, uint32_t mask)
{
	p->val = 0;
	p->val = mask;
	p->page_frame = ((uint32_t)addr) >> 12;
}

void make_pte_mask(PTE *p, void *addr, uint32_t mask)
{
	p->val = 0;
	p->val = mask;
	p->page_frame = ((uint32_t)addr) >> 12;
}

inline uint32_t get_pte_ind(uint32_t n) {
    return ((n & 0x3ff000)>>12); // 0000 0000 0011 1111 1111 0000 0000 0000
}

uintptr_t va_pte(PDE* p)
{
	uintptr_t tem = p->page_frame;
	tem = tem << 12;
	return (uintptr_t)pa_to_va(tem);
}

uintptr_t va_byte(PTE *p)
{
	uintptr_t tem = p->page_frame;
	tem = tem << 12;
	return (uintptr_t)pa_to_va(tem);
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

