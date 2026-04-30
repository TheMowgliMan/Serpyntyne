#ifndef MM_HEAP_H_
#define MM_HEAP_H_

#include <stdint.h>
#include <stddef.h>

struct alloc_page;

struct alloc_page {
    uint8_t *current_page;
    size_t free_offset;
    uint8_t size;

    struct alloc_page *next;
};

void heap_init(void);

void *krmalloc(size_t size);
void *krcalloc(size_t size);
void *xkrmalloc(size_t size);

#endif
