#ifndef VMM_H_
#define VMM_H_

#include <mm/pmm.h>
#include <paging.h>

#include <stdint.h>

#define KERNEL_ENTRY_POINT 0xffffffff80000000

extern struct pagemap *kernel_page_table;

void paging_init(void);

int map_multiple_pages(struct pagemap *map, uint64_t pages, uintptr_t virt_addr, struct physFrame phys_addr, uint64_t flags);

void *allocate_random_and_map(struct pagemap *map, struct randomInstance *ri, size_t bytes, uintptr_t virt_addr, uint64_t flags);

#endif
