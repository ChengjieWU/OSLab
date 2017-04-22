#include "common.h"
#include "string.h"

#include "memory.h"
#include "mmu.h"
#include "irq.h"
#include "process.h"

#include "device/video.h"

typedef struct page_info {
	bool free;
	int cited;
	uint32_t id;
	struct page_info* next;
} page_info;

page_info page_mm[NR_PAGE];
page_info* page_free_list;


#define ALLOC_ZERO 1

static inline physaddr_t page2pa(struct page_info *pp) 
{
	return pp->id << PGSHIFT;
}

static inline page_info* pa2page(physaddr_t pa)
{
	/* In kernel, specifically, we can access everything via va and pa. */
	if (PGNUM(pa) >= NR_PAGE) panic("Invalid pa!\n");
	return &page_mm[PGNUM(pa)];
}


void init_mm()
{
	int i = 0;
	for (; i < NR_KERNEL_PAGE; i++)
	{
		page_mm[i].free = false;
		page_mm[i].cited = 1;
		page_mm[i].id = i;
		page_mm[i].next = NULL;
	}
	for (; i < NR_PAGE; i++)
	{
		page_mm[i].free = true;
		page_mm[i].cited = 0;
		page_mm[i].id = i;
		page_mm[i].next = page_free_list;
		page_free_list = &page_mm[i];
	}
}

static page_info* page_alloc(int alloc_flags)
{
	if (page_free_list == NULL) return NULL;
	page_info* p = page_free_list;
	page_free_list = page_free_list->next;
	p->free = false;
	p->next = NULL;
	if (alloc_flags & ALLOC_ZERO)
	{
		physaddr_t phy_addr = page2pa(p);
		/* Always use va after paging! Cause user programs cannot access pa! */
		memset((uint8_t*)pa_to_va(phy_addr), 0, PAGE_SIZE);
	}
	return p;
}

/*static void page_free(page_info* pp)
{
	pp->free = true;
	pp->next = page_free_list;
	page_free_list = pp;
}

static void page_dec_cited(page_info* pp)
{
	if (--pp->cited == 0)
		page_free(pp);
}*/

static PTE* pgdir_walk(PDE* pgdir, const void *va, bool create)
{
	if (pgdir[PDX(va)].present == 0)
	{
		if (create == false) return NULL;
		else 
		{
			page_info* p = page_alloc(ALLOC_ZERO);
			if (p == NULL) return NULL;
			else 
			{
				p->cited++;
				physaddr_t phy_addr = page2pa(p);
				/* Since there're two levels of page_translation, we leave page protection to the second level. */
				/* So PDE terms are always set to user_mode. */
				make_pde(&pgdir[PDX(va)], (void*)phy_addr);
			}
		}
	}
	PTE* pgtable = pa_to_va(pgdir[PDX(va)].page_frame << PGSHIFT);
	return &pgtable[PTX(va)];
}

/* Used only for map, don't allocate physical frame. */
static void boot_map_region(PDE *pgdir, uintptr_t va, unsigned long size, physaddr_t pa, int perm)
{
	if (size % PAGE_SIZE != 0) panic("size is not the multiple of PAGE_SIZE!\n");
	if (va % PAGE_SIZE != 0) panic("va is not the multiple of PAGE_SIZE!\n");
	if (pa % PAGE_SIZE != 0) panic("pa is not the multiple of PAGE_SIZE!\n");
	uintptr_t limit = va + size;
	while (va < limit)
	{
		PTE* pgtable = pgdir_walk(pgdir, (const void*)va, true);
		make_pte_mask(pgtable, (void*)pa, perm | PTE_P);
		va += PAGE_SIZE;
		pa += PAGE_SIZE;
	}
}

//
// Map the physical page 'pp' at virtual address 'va'.
// The permissions (the low 12 bits) of the page table entry
// should be set to 'perm|PTE_P'.
//
// Requirements
//   - If there is already a page mapped at 'va', it should be page_removed().
//   - If necessary, on demand, a page table should be allocated and inserted
//     into 'pgdir'.
//   - pp->pp_ref should be incremented if the insertion succeeds.
//   - The TLB must be invalidated if a page was formerly present at 'va'.
//
// Corner-case hint: Make sure to consider what happens when the same
// pp is re-inserted at the same virtual address in the same pgdir.
// However, try not to distinguish this case in your code, as this
// frequently leads to subtle bugs; there's an elegant way to handle
// everything in one code path.
//
// RETURNS:
//   0 on success
//   -E_NO_MEM, if page table couldn't be allocated
//
// Hint: The TA solution is implemented using pgdir_walk, page_remove,
// and page2pa.
//

/* This is a currently simplified version. */
static int page_insert(PDE *pgdir, page_info* pp, void *va, int perm)
{
	PTE* pte = pgdir_walk(pgdir, (const void*)va, true);
	if (pte->present == 1) return -1;
	else
	{
		pp->cited++;
		physaddr_t phy_addr = page2pa(pp);
		make_pte_mask(pte, (void*)phy_addr, perm | PTE_P);
		return 0;
	}
}

uint32_t request_for_page()
{
	page_info* pp = page_alloc(ALLOC_ZERO);
	if (pp == NULL) panic("No free pages!\n");
	pp->cited++;
	physaddr_t phy_addr = page2pa(pp);
	return (uint32_t)pa_to_va(phy_addr);
}

PDE* init_updir()
{
	/* Allocate a new physical page to store PDE table. */
	page_info* pp = page_alloc(ALLOC_ZERO);
	if (pp == NULL) panic("No free pages!\n");
	pp->cited++;
	physaddr_t phy_addr = page2pa(pp);
	
	PDE* updir = pa_to_va(phy_addr);
	
	/* Map kernel space to user space. */
	/* IMPORTANT NOTE: PTE must be set to kernel mode to enable page protection! */
	boot_map_region(updir, KOFFSET, PHY_MEM, 0, PTE_W);
	
	/* ###### As for vmem, since SCR_SIZE <= PD_SIZE, so it is in one pde term. I simplify it here. ###### */
	if (SCR_SIZE > PD_SIZE) panic("The situation needs handling!\n");
	boot_map_region(updir, VMEM_ADDR, PD_SIZE, VMEM_ADDR, PTE_W);
	
	/* This manually version to load all pages will never be used. */
	/*
	uint32_t aa = 0x8048000;
	int i = 0;
	for (; i < 4096; i++)
	{
		page_insert(updir, page_alloc(ALLOC_ZERO), (void*)(aa + i * PAGE_SIZE), PTE_U | PTE_W);
	}*/

	return updir;
}


void page_fault_handler(TrapFrame* tf)
{
	if (tf->error_code & FEC_PR) panic("Page-level protection violation at eip = %x!\n", tf->eip);
	if (tf->error_code & FEC_U) 
	{
		if (tf->error_code & FEC_WR)
		{
			uintptr_t address = read_cr2();
			/* The virtual space for user programs is 0x0 - 0xbfffffff, right below kernel, 3GB in total. */
			if (address >= KOFFSET) panic("User_mode writing page fault at eip = %x!\n", tf->eip);
			page_insert(current->pgdir, page_alloc(ALLOC_ZERO), (void*)address, PTE_U | PTE_W);
		}
		else
		{
			panic("User_mode reading page fault at eip = %x!\n", tf->eip);
		}
	}
	else 
	{
		if (tf->error_code & FEC_WR)
		{
			uintptr_t address = read_cr2();
			/* This should only be used when loading user programs. */
			/* The virtual space for user programs is 0x0 - 0xbfffffff, right below kernel, 3GB in total. */
			if (address >= KOFFSET) panic("User_mode page fault at eip = %x!\n", tf->eip);
			page_insert(current->pgdir, page_alloc(ALLOC_ZERO), (void*)address, PTE_U | PTE_W);
		}
		else
		{
			panic("Kernel_mode reading page fault at eip = %x!\n", tf->eip);
		}
	}	
}
