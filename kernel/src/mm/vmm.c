#include <mm/vmm.h>
#include <mm/pmm.h>
#include <terminal.h>
#include <stdint.h>
#include <stdbool.h>
#include <paging.h>
#include <archutil/defines.h>
#include <util/liminereq.h>
#include <util/atomics.h>
#include <util/align.h>
#include <util/panic.h>

struct pagemap kpt;
struct pagemap *kernel_page_table;

spinlock_t kernel_vmm_lock;

void paging_init(void)
{
    klog(LOG_PROC, "Starting Virtual Memory Manager...\r\n");

    initSpinlock(&kernel_vmm_lock);

    struct physFrame tmpf = allocate_page_random(NULL);
    uint64_t *kernel_top_level_paging_directory = (uint64_t *)(tmpf.phys_addr + hhdm_response->offset);

    kpt = generate_pagemap(kernel_top_level_paging_directory, &kernel_vmm_lock);
    kernel_page_table = &kpt;

#ifdef DEBUG
    klog(LOG_INFO, "Mapping HHDM...\r\n");
#endif

    for (uint64_t i = 0; i < memmap_response->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap_response->entries[i];

        if (entry->type == LIMINE_MEMMAP_RESERVED
            || entry->type == LIMINE_MEMMAP_BAD_MEMORY)
        {
            continue;
        }


        uintptr_t floor = (uintptr_t)(ALIGN_DOWN(entry->base, PAGE_SIZE));
        uintptr_t cieling = (uintptr_t)(ALIGN_UP(entry->base + entry->length, PAGE_SIZE));


        uint64_t flags = MAP_WRITABLE | MAP_EXECUTABLE | MAP_KERNEL;
        if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER)
            flags |= MAP_WRITETHROUGH | MAP_CACHEDISABLE;

        uintptr_t addr = floor;


        while (addr < cieling)
        {
            if (addr + LARGE_PAGE_SIZE <= cieling && IS_ALIGNED(addr, LARGE_PAGE_SIZE))
            {
                tmpf = gen_frame(addr, LARGE_PAGE_SIZE_EXPONENT, false);

                map_page(kernel_page_table, (uintptr_t)(addr + hhdm_response->offset), tmpf, flags);
                addr += LARGE_PAGE_SIZE;
            }
            else if (addr + PAGE_SIZE <= cieling && IS_ALIGNED(addr, PAGE_SIZE))
            {
                tmpf = gen_frame(addr, PAGE_SIZE_EXPONENT, false);

                map_page(kernel_page_table, (uintptr_t)(addr + hhdm_response->offset), tmpf, flags);
                addr += PAGE_SIZE;
            }
        }
    }
}
