#include "x86.h"
#include "memory.h"
#include "string.h"

#include "device/video.h"

#ifdef IA32_PAGE

void init_vmem_space()
{
	/* Since SCR_SIZE < PT_SIZE, so it is in one pde term. I simplify it here. ##### */
	PDE* kpdir_t = get_kpdir();
	PTE* kptable_t = get_kptable();
	PTE* vptable = &kptable_t[PHY_MEM / PAGE_SIZE];
	make_pde(&kpdir_t[(VMEM_ADDR >> 22) & 0x3ff], va_to_pa(vptable));
	uint32_t ptable_idx;
	for (ptable_idx = 0; ptable_idx < NR_PTE; ptable_idx++)
		make_pte(&vptable[ptable_idx], (void*)(VMEM_ADDR + ptable_idx * PAGE_SIZE));
}

void vmem_writing_test() 
{
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		buf[i] = i;
	}
}

void vmem_reading_test() 
{
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		assert(buf[i] == i);
	}
}

#endif
