#include "common.h"
#include "string.h"
#include "memory.h"

typedef struct page_info {
	bool free;
	int cited;
	uint32_t id;
	struct page_info* next;
} page_info;

page_info page_mm[NR_PAGE];
page_info* page_free_list;


///////////////////////////////////////////////////////

enum {
	// For page_alloc, zero the returned physical page.
	ALLOC_ZERO = 1<<0,
};

static inline physaddr_t page2pa(struct page_info *pp)
{
	return pp->id << 12;
}

static inline page_info* pa2page(physaddr_t pa)
{
	uint32_t pagenum = pa >> 12;
	if (pagenum >= NR_PAGE) panic("Invalid pa!\n");
	return &page_mm[pagenum];
}

///////////////////////////////////////////////////////



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

page_info* page_alloc(int alloc_flags)
{
	if (page_free_list == NULL) panic("No free pages!\n");
	page_info* p = page_free_list;
	page_free_list = page_free_list->next;
	p->free = false;
	p->next = NULL;
	if (alloc_flags & ALLOC_ZERO)
	{
		physaddr_t pa = page2pa(p);
		memset((uint8_t*)pa, 0, PAGE_SIZE);
	}
	return p;
}

void page_free(page_info* pp)
{
	pp->free = true;
	pp->next = page_free_list;
	page_free_list = pp;
}

void page_dec_cited(page_info* pp)
{
	if (--pp->cited == 0)
		page_free(pp);
}


