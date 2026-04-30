#include <mm/heap.h>

#include <archutil/defines.h>

#include <util/liminereq.h>
#include <util/align.h>
#include <util/panic.h>

#include <mm/pmm.h>
#include <mm/vmm.h>

#include <paging.h>
#include <memory.h>
#include <terminal.h>

#include <stdint.h>
#include <stddef.h>

struct alloc_page c;
struct alloc_page kvmalloc_head;

void heap_init(void)
{
    klog(LOG_PROC, "Starting heap...\r\n");

    c.next = NULL;

    c.current_page = (uint8_t*)allocate_random_and_map(kernel_page_table,
                                                       NULL,
                                                       LARGE_PAGE_SIZE,
                                                       kernel_page_table->heap_start,
                                                       MAP_NEWMAP | MAP_READABLE | MAP_WRITABLE | MAP_KERNEL);
    c.size = LARGE_PAGE_SIZE_EXPONENT;

    kernel_page_table->heap_offset += LARGE_PAGE_SIZE;
    c.free_offset = 0;

    klog(LOG_SUCCESS, "Heap started!\r\n");
}

void *krmalloc(size_t size)
{
    if (page_exponent_to_standard(c.size) - c.free_offset > size)
    {
        size_t idx = c.free_offset;
        c.free_offset += ALIGN_UP(size, ARCH_WIDTH);

        return (void*)(&c.current_page[idx]);
    }
    else if (size > PAGE_SIZE)
    {
        void *ret = (void*)allocate_random_and_map(kernel_page_table, NULL, size,
                                                   kernel_page_table->heap_start + kernel_page_table->heap_offset,
                                                   MAP_NEWMAP | MAP_READABLE | MAP_WRITABLE | MAP_KERNEL);

        kernel_page_table->heap_offset += ALIGN_UP(size, PAGE_SIZE);
        return ret;
    }
    else
    {
        c.current_page = (uint8_t*)allocate_random_and_map(kernel_page_table,
                                                           NULL,
                                                           LARGE_PAGE_SIZE,
                                                           kernel_page_table->heap_start + kernel_page_table->heap_offset,
                                                           MAP_NEWMAP | MAP_READABLE | MAP_WRITABLE | MAP_KERNEL);
        kernel_page_table->heap_offset += LARGE_PAGE_SIZE;
        c.free_offset = ALIGN_UP(size, ARCH_WIDTH);

        return (void*)(&c.current_page[0]);
    }

    return NULL;
}

void *krcalloc(size_t size)
{
    void *ret = krmalloc(size);
    memset(ret, 0, size);
    return ret;
}

void *xkrmalloc(size_t size)
{
    void *ret = krmalloc(size);
    if (ret == NULL || ret == 0)
    {
        exception(OUT_OF_MEMORY, OOM_XKRMALLOC_CALL, 0);
    }
    return ret;
}
