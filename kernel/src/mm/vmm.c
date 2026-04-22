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

void paging_init(void)
{
    kprintf("Starting Virtual Memory Manager...\r\n");

    struct physFrame tmpf= allocate_page_random(NULL);
    kernel_page_table = tmpf.phys_addr;

#ifdef DEBUG
    kprintf("Mapping HHDM...\r\n");
#endif
}
