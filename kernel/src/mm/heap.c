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

struct free_block *kvmalloc_head;

spinlock_t khl;
spinlock_t *kernel_heap_lock;

void heap_init(void)
{
    klog(LOG_PROC, "Starting heap...\r\n");

    c.next = NULL;

    c.current_page = (uint8_t*)allocate_random_and_map(kernel_page_table,
                                                       NULL,
                                                       ARCH_HEAP_ALLOCATE_SIZE,
                                                       kernel_page_table->heap_start,
                                                       MAP_NEWMAP | MAP_READABLE | MAP_WRITABLE | MAP_KERNEL);
    c.size = ARCH_HEAP_ALLOCATE_SIZE_EXPONENT;

    kernel_page_table->heap_offset += ARCH_HEAP_ALLOCATE_SIZE;
    c.free_offset = 0;

    // availability of krmalloc and friends begins here

    kernel_heap_lock = &khl;
    initSpinlock(&khl);

    kvmalloc_head = (struct free_block*)krcalloc(sizeof(struct free_block));
    kvmalloc_head->data = krmalloc(ARCH_HEAP_ALLOCATE_SIZE);
    kvmalloc_head->size = ARCH_HEAP_ALLOCATE_SIZE;
    kvmalloc_head->free_offset = 0;
    kvmalloc_head->next = NULL;

    // now all malloc varieties are available

    klog(LOG_SUCCESS, "Heap started!\r\n");
}

void *krmalloc(size_t size)
{
    acquireSpinlock(kernel_heap_lock, 0);

    if (page_exponent_to_standard(c.size) - c.free_offset > size)
    {
        size_t idx = c.free_offset;
        c.free_offset += ALIGN_UP(size, ARCH_WIDTH);

        releaseSpinlock(kernel_heap_lock);
        return (void*)(&c.current_page[idx]);
    }
    else if (size > ARCH_HEAP_MAP_IF_LARGER_SIZE)
    {
        void *ret = (void*)allocate_random_and_map(kernel_page_table, NULL, size,
                                                   kernel_page_table->heap_start + kernel_page_table->heap_offset,
                                                   MAP_NEWMAP | MAP_READABLE | MAP_WRITABLE | MAP_KERNEL);

        kernel_page_table->heap_offset += ALIGN_UP(size, PAGE_SIZE); // allocate_random_and_map() always aligns up to the arch page size

        releaseSpinlock(kernel_heap_lock);
        return ret;
    }
    else
    {
        if (page_exponent_to_standard(c.size) - c.free_offset > ARCH_WIDTH)
        {
            struct free_block *new = (struct free_block*)krmalloc(sizeof(struct free_block));
            new->data = (void*)(c.current_page + c.free_offset);
            new->size = page_exponent_to_standard(c.size) - c.free_offset;
            new->free_offset = 0;

            new->next = kvmalloc_head;
            kvmalloc_head = new;
        }

        c.current_page = (uint8_t*)allocate_random_and_map(kernel_page_table,
                                                           NULL,
                                                           ARCH_HEAP_ALLOCATE_SIZE,
                                                           kernel_page_table->heap_start + kernel_page_table->heap_offset,
                                                           MAP_NEWMAP | MAP_READABLE | MAP_WRITABLE | MAP_KERNEL);
        kernel_page_table->heap_offset += ARCH_HEAP_ALLOCATE_SIZE;
        c.free_offset = ALIGN_UP(size, ARCH_WIDTH);

        releaseSpinlock(kernel_heap_lock);
        return (void*)(&c.current_page[0]);
    }

    releaseSpinlock(kernel_heap_lock);

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

void *kvmalloc(size_t size)
{
    acquireSpinlock(kernel_heap_lock, 0);

    uint8_t *ret = NULL;
    struct free_block *prev = NULL;
    for (struct free_block *cur = kvmalloc_head; cur != NULL; cur = cur->next)
    {
        if (cur->size - cur->free_offset > size)
        {
            if (prev != NULL)
                prev->next = cur->next;

            ret = (uint8_t*)cur->data + cur->free_offset;
            cur->free_offset += ALIGN_UP(size, ARCH_WIDTH);
            break;
        }

        prev = cur;
    }

    if (ret == NULL || ret == 0)
    {
        ret = krmalloc(size);
    }

    releaseSpinlock(kernel_heap_lock);

    return (void*)ret;
}

void *kvcalloc(size_t size)
{
    void *ret = kvmalloc(size);
    memset(ret, 0, size);
    return ret;
}

void *xkvmalloc(size_t size)
{
    void *ret = krmalloc(size);
    if (ret == NULL || ret == 0)
    {
        exception(OUT_OF_MEMORY, OOM_XKVMALLOC_CALL, 0);
    }
    return ret;
}

void free_sized(void *ptr, size_t size)
{
    struct free_block *b = NULL;
    if (size > sizeof(struct free_block))
        b = (struct free_block*)ptr;
    else
        b = krmalloc(sizeof(struct free_block));

    b->data = ptr;
    b->size = size;
    b->free_offset = 0;

    b->next = kvmalloc_head;
    kvmalloc_head = b;

    ptr = NULL;
}

void *knock_a_few_bytes_off_the_old_heap_block(uintptr_t phys_base, uintptr_t offset)
{
    acquireSpinlock(kernel_heap_lock, 0);

    uintptr_t floor = ALIGN_DOWN(phys_base, PAGE_SIZE);
    uintptr_t cieling = ALIGN_UP(phys_base + offset, PAGE_SIZE);

    uint64_t pages_mapped = 0;
    for (uint64_t page = 0; page * PAGE_SIZE < cieling; page++)
    {
        map_page(kernel_page_table,
                 kernel_page_table->heap_start + kernel_page_table->heap_offset + page * PAGE_SIZE,
                 gen_frame(phys_base + page * PAGE_SIZE, PAGE_SIZE_EXPONENT, false),
                 MAP_NEWMAP | MAP_READABLE | MAP_WRITABLE | MAP_KERNEL);

        pages_mapped++;
    }

    kernel_page_table->heap_offset += pages_mapped * PAGE_SIZE;

    releaseSpinlock(kernel_heap_lock);
}
