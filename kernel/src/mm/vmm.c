#include <mm/vmm.h>
#include <terminal.h>
#include <stdint.h>
#include <paging.h>
#include <archutil/defines.h>
#include <util/liminereq.h>
#include <util/atomics.h>
#include <util/align.h>
#include <util/panic.h>

uintptr_t kernel_page_table;
uint64_t *kpt_ptr;

void paging_init(void)
{
    klog(LOG_PROC, "Starting Virtual Memory Manager...\r\n");

    struct physFrame tmpf= allocate_page_random(NULL);
    kernel_page_table = tmpf.phys_addr;

    kpt_ptr = (uint64_t*)(kernel_page_table + hhdm_response->offset);

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
    }
}
