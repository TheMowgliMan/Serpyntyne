#ifndef MM_HEAP_H_
#define MM_HEAP_H_

#include <stdint.h>
#include <stddef.h>

struct alloc_page;

struct free_block;

struct alloc_page {
    uint8_t *current_page;
    size_t free_offset;
    uint8_t size;

    struct alloc_page *next;
};

struct free_block {
    void *data;
    size_t size;
    size_t free_offset;

    struct free_block *next;
};

void heap_init(void);

void *krmalloc(size_t size);
void *krcalloc(size_t size);
void *xkrmalloc(size_t size);

void *kvmalloc(size_t size);

#endif
