#ifdef IA32_PAGEPAGE

#include "x86.h"
#include "mmu.h"

#define PAGE_SIZE					4096
#define NR_PDE						1024
#define NR_PTE						1024
#define PAGE_MASK					(4096 - 1)
#define PT_SIZE						((NR_PTE) * (PAGE_SIZE))

#define align_to_page              __attribute((aligned(PAGE_SIZE)))

typedef union PageDirectoryEntry {
	struct {
		uint32_t present             : 1;
		uint32_t read_write          : 1; 
		uint32_t user_supervisor     : 1;
		uint32_t page_write_through  : 1;
		uint32_t page_cache_disable  : 1;
		uint32_t accessed            : 1;
		uint32_t pad0                : 6;
		uint32_t page_frame          : 20;
	};
	uint32_t val;
} PDE;


typedef union PageTableEntry {
	struct {
		uint32_t present             : 1;
		uint32_t read_write          : 1;
		uint32_t user_supervisor     : 1;
		uint32_t page_write_through  : 1;
		uint32_t page_cache_disable  : 1;
		uint32_t accessed            : 1;
		uint32_t dirty               : 1;
		uint32_t pad0                : 1;
		uint32_t global              : 1;
		uint32_t pad1                : 3;
		uint32_t page_frame          : 20;
	};
	uint32_t val;
} PTE;

static PDE kpdir[NR_PDE] align_to_page;						
static PTE kptable[PHY_MEM / PAGE_SIZE] align_to_page;		

PDE* get_kpdir() { return kpdir; }


/* set up page tables for kernel */
void init_page(void) {
	CR0 cr0;
	CR3 cr3;
	PDE *pdir = (PDE *)va_to_pa(kpdir);				//point to the PDE in the hwaddr
	PTE *ptable = (PTE *)va_to_pa(kptable);			//point to the PTE in the hwaddr
	uint32_t pdir_idx;

	/* make all PDEs invalid */
	memset(pdir, 0, NR_PDE * sizeof(PDE));

	/* fill PDEs */
	for (pdir_idx = 0; pdir_idx < PHY_MEM / PT_SIZE; pdir_idx ++) {
		pdir[pdir_idx].val = make_pde(ptable);
		pdir[pdir_idx + KOFFSET / PT_SIZE].val = make_pde(ptable);

		ptable += NR_PTE;
	}

	/* fill PTEs */

	/* We use inline assembly here to fill PTEs for efficiency.
	 * If you do not understand it, refer to the C code below.
	 */	
	
	asm volatile ("std;\
	 1: stosl;\
		subl %0, %%eax;\
		jge 1b;\
		cld" : :
		"i"(PAGE_SIZE), "a"((PHY_MEM - PAGE_SIZE) | 0x7), "D"(ptable - 1));


	/*
		===== referenced code for the inline assembly above =====

		uint32_t pframe_addr = PHY_MEM - PAGE_SIZE;
		ptable --;

		// fill PTEs reversely
		for (; pframe_addr >= 0; pframe_addr -= PAGE_SIZE) {
			ptable->val = make_pte(pframe_addr);
			ptable --;
		}
	*/


	/* make CR3 to be the entry of page directory */
	cr3.val = 0;
	cr3.page_directory_base = ((uint32_t)pdir) >> 12;
	write_cr3(cr3.val);

	/* set PG bit in CR0 to enable paging */
	cr0.val = read_cr0();
	cr0.paging = 1;
	write_cr0(cr0.val);
}

#endif

