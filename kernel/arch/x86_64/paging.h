#ifndef PAGING_H_
#define PAGING_H_

#include <stdint.h>
#include <stdbool.h>

#include <archutil/asmstubs.h>
#include <archutil/defines.h>

#include <util/atomics.h>
#include <util/panic.h>
#include <util/liminereq.h>

#include <mm/pmm.h>

#define PAGE_TABLE_ADDRESS_MASK 0xFFF0000000000FFFull
#define PAGE_TABLE_NX_BIT 0x8000000000000000ull
#define PAGE_TABLE_FLAGS_MASK 0xFFFull
#define PAGE_TABLE_PK_BITS 0x7800000000000000ull

#define PT_DISABLE_RESERVED_MASK 0xFFF0FFFFFFFFFFFFull
#define PDPT_PD_DISABLE_RESERVED_MASK 0xFFF0FFFFFFFFFEBF;
#define PML5PML4_DISABLE_RESERVED_MASK 0xFFF0FFFFFFFFFE3Full;

#define PAGE_FLAG_PRESENT (1ull)
#define PAGE_FLAG_READWRITE (1ull << 1)
#define PAGE_FLAG_USERSUPERVISOR (1ull << 2)
#define PAGE_FLAG_PAGE_LEVEL_WRITE_THROUGH (1ull << 3)
#define PAGE_FLAG_PAGE_LEVEL_CACHE_DISABLE (1ull << 4)
#define PAGE_FLAG_ACCESSED (1ull << 5)
#define PAGE_FLAG_DIRTY (1ull << 6)
#define PAGE_FLAG_PAGE_ATTRIBUTE_TABLE (1ull << 7)
#define PAGE_FLAG_GLOBAL (1ull << 8)
#define PAGE_FLAG_AVAILABLE_1 (1ull << 9)
#define PAGE_FLAG_AVAILABLE_2 (1ull << 10)
#define PAGE_FLAG_AVAILABLE_3 (1ull << 11)

#define PAGE_DIRECTORY_PRESENT (1ull)
#define PAGE_DIRECTORY_READWRITE (1ull << 1)
#define PAGE_DIRECTORY_USERSUPERVISOR (1ull << 2)
#define PAGE_DIRECTORY_PAGE_LEVEL_WRITE_THROUGH (1ull << 3)
#define PAGE_DIRECTORY_PAGE_LEVEL_CACHE_DISABLE (1ull << 4)
#define PAGE_DIRECTORY_ACCESSED (1ull << 5)
#define PAGE_DIRECTORY_RESERVED1 (1ull << 6)
#define PAGE_DIRECTORY_SIZE_BIT (1ull << 7)
#define PAGE_DIRECTORY_RESERVED3 (1ull << 8)
#define PAGE_DIRECTORY_AVAILABLE_1 (1ull << 9)
#define PAGE_DIRECTORY_AVAILABLE_2 (1ull << 10)
#define PAGE_DIRECTORY_AVAILABLE_3 (1ull << 11)

#define PAGE_TABLE_ENTRY_SIZE 8
#define PAGE_TABLE_ENTRY_COUNT 512

#define ACCEPTABLE_PAGE_SIZES 0x201000ull

#define MAP_WRITABLE PAGE_FLAG_READWRITE
#define MAP_READABLE PAGE_FLAG_PRESENT // Pages are always readable on x86-64
#define MAP_EXECUTABLE 0 // Pages are executable by default on x86-64
#define MAP_NOEXECUTE PAGE_TABLE_NX_BIT
#define MAP_USER PAGE_FLAG_USERSUPERVISOR
#define MAP_KERNEL 0 // Pages are kernel by default on x86-64
#define MAP_CACHEDISABLE PAGE_FLAG_PAGE_LEVEL_CACHE_DISABLE
#define MAP_WRITETHROUGH PAGE_FLAG_PAGE_LEVEL_WRITE_THROUGH
#define MAP_LARGE PAGE_DIRECTORY_SIZE_BIT
#define MAP_NEWMAP PAGE_FLAG_PRESENT

#define NEW_PAGE_TABLE_FLAGS (MAP_NEWMAP | MAP_USER | MAP_KERNEL | MAP_READABLE | MAP_EXECUTABLE | MAP_WRITABLE)

struct pagemap {
    uint64_t *top_level;
    spinlock_t *map_lock;

    uintptr_t kernel_start;
    uintptr_t kernel_end;

    uintptr_t fs_end; // This and...
    uintptr_t gs_end; // this are available markers to be defined by the kernel for its purposes

    uintptr_t heap_start; // Virt addr of the heap
    size_t heap_offset; // Length of the heap on the virtual address space
};

inline void __attribute__((always_inline)) set_table_base_address(uint64_t *table, uint64_t addr)
{
    *table &= PAGE_TABLE_ADDRESS_MASK;
    *table |= (addr & ~PAGE_TABLE_ADDRESS_MASK);
}

inline uint64_t __attribute__((always_inline)) get_table_base_address(uint64_t *table)
{
    return (*table & ~PAGE_TABLE_ADDRESS_MASK);
}

inline void __attribute__((always_inline)) set_table_nx(uint64_t *table)
{
    *table |= PAGE_TABLE_NX_BIT;
}

inline void __attribute__((always_inline)) clear_table_nx(uint64_t *table)
{
    *table &= ~PAGE_TABLE_NX_BIT;
}

inline bool __attribute__((always_inline)) get_table_nx(uint64_t *table)
{
    if (*table & PAGE_TABLE_NX_BIT) return true; else return false;
}

inline void __attribute__((always_inline)) set_table_flags(uint64_t *table, uint16_t flags)
{
    *table |= (uint64_t)flags & PAGE_TABLE_FLAGS_MASK;
}

inline void __attribute__((always_inline)) clear_table_flags(uint64_t *table, uint16_t flags)
{
    *table &= ~PAGE_TABLE_FLAGS_MASK | ~(uint64_t)flags;
}

inline uint16_t __attribute__((always_inline)) get_table_flags(uint64_t *table)
{
    return (uint16_t)(*table & PAGE_TABLE_FLAGS_MASK);
}

struct pagemap generate_pagemap(uint64_t *top_level, spinlock_t *spinlock);

int map_page(struct pagemap *map, uintptr_t virt_addr, struct physFrame phys_addr, uint64_t flags);

void check_page_align(uintptr_t virt_addr, uintptr_t phys_addr, uint64_t align);

#endif
