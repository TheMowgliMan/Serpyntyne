#ifndef PAGING_H_
#define PAGING_H_

/* This is a header-only implementation because I'm too lazy to create the other file :P */

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

#define PAGE_FLAG_PRESENT 1ull
#define PAGE_FLAG_READWRITE 1ull << 1
#define PAGE_FLAG_USERSUPERVISOR 1ull << 2
#define PAGE_FLAG_PAGE_LEVEL_WRITE_THROUGH 1ull << 3
#define PAGE_FLAG_PAGE_LEVEL_CACHE_DISABLE 1ull << 4
#define PAGE_FLAG_ACCESSED 1ull << 5
#define PAGE_FLAG_DIRTY 1ull << 6
#define PAGE_FLAG_PAGE_ATTRIBUTE_TABLE 1ull << 7
#define PAGE_FLAG_GLOBAL 1ull << 8
#define PAGE_FLAG_AVAILABLE_1 1ull << 9
#define PAGE_FLAG_AVAILABLE_2 1ull << 10
#define PAGE_FLAG_AVAILABLE_3 1ull << 11

#define PAGE_DIRECTORY_PRESENT 1ull
#define PAGE_DIRECTORY_READWRITE 1ull << 1
#define PAGE_DIRECTORY_USERSUPERVISOR 1ull << 2
#define PAGE_DIRECTORY_PAGE_LEVEL_WRITE_THROUGH 1ull << 3
#define PAGE_DIRECTORY_PAGE_LEVEL_CACHE_DISABLE 1ull << 4
#define PAGE_DIRECTORY_ACCESSED 1ull << 5
#define PAGE_DIRECTORY_RESERVED1 1ull << 6
#define PAGE_DIRECTORY_SIZE_BIT 1ull << 7
#define PAGE_DIRECTORY_RESERVED3 1ull << 8
#define PAGE_DIRECTORY_AVAILABLE_1 1ull << 9
#define PAGE_DIRECTORY_AVAILABLE_2 1ull << 10
#define PAGE_DIRECTORY_AVAILABLE_3 1ull << 11

#define PAGE_TABLE_ENTRY_SIZE 8
#define PAGE_TABLE_ENTRY_COUNT 512

#define ACCEPTABLE_PAGE_SIZES 0x201000ull

#define MAP_WRITABLE PAGE_FLAG_READWRITE
#define MAP_EXECUTABLE (~PAGE_TABLE_NX_BIT)
#define MAP_USER PAGE_FLAG_USERSUPERVISOR
#define MAP_KERNEL (~PAGE_FLAG_USERSUPERVISOR)
#define MAP_CACHEDISABLE PAGE_FLAG_PAGE_LEVEL_CACHE_DISABLE
#define MAP_WRITETHROUGH PAGE_FLAG_PAGE_LEVEL_WRITE_THROUGH
#define MAP_LARGE PAGE_DIRECTORY_SIZE_BIT

struct pagemap {
    uint64_t *top_level;
    spinlock_t *map_lock;
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

struct pagemap generate_pagemap(uint64_t *top_level, spinlock_t *spinlock)
{
    struct pagemap pm;
    pm.top_level = top_level;
    pm.map_lock = spinlock;
    return pm;
}

int map_page(struct pagemap *map, uintptr_t virt_addr, struct physFrame phys_addr, uint64_t flags)
{
    if (!(((1ull << phys_addr.size) * PAGE_SIZE) & ACCEPTABLE_PAGE_SIZES)) exception(ILLEGAL_PAGE_MAP_SIZE, (int64_t)phys_addr.size, (int64_t)virt_addr);

    acquireSpinlock(map->map_lock, 0);

    // Thanks EvalynOS :P
    uint16_t pml1i = (virt_addr >> 12) & 0x1ff;
    uint16_t pml2i = (virt_addr >> 21) & 0x1ff;
    uint16_t pml3i = (virt_addr >> 30) & 0x1ff;
    uint16_t pml4i = (virt_addr >> 39) & 0x1ff;

    if (!(map->top_level[pml4i] & PAGE_DIRECTORY_PRESENT))
    {
        struct physFrame new_frame = allocate_page_generic(4);
        map->top_level[pml4i] = (uint64_t)(new_frame.phys_addr & ~PAGE_TABLE_ADDRESS_MASK);
        set_table_flags(&map->top_level[pml4i], PAGE_DIRECTORY_PRESENT | PAGE_DIRECTORY_READWRITE | PAGE_DIRECTORY_USERSUPERVISOR);
    }

    uint64_t *pml3v = (uint64_t*)((map->top_level[pml4i] & ~PAGE_TABLE_ADDRESS_MASK) + hhdm_response->offset);
    if (!(pml3v[pml3i] & PAGE_DIRECTORY_PRESENT))
    {
        struct physFrame new_frame = allocate_page_generic(6);
        pml3v[pml3i] = (uint64_t)(new_frame.phys_addr & ~PAGE_TABLE_ADDRESS_MASK);
        set_table_flags(&pml3v[pml3i], PAGE_DIRECTORY_PRESENT | PAGE_DIRECTORY_READWRITE | PAGE_DIRECTORY_USERSUPERVISOR);
    }

    uint64_t *pml2v = (uint64_t*)((pml3v[pml3i] & ~PAGE_TABLE_ADDRESS_MASK) + hhdm_response->offset);

    if (phys_addr.size == LARGE_PAGE_SIZE_EXPONENT)
    {
        pml2v[pml2i] = (uint64_t)(phys_addr.phys_addr & ~PAGE_TABLE_ADDRESS_MASK);
        set_table_flags((uint64_t*)(&pml2v[pml2i]), PAGE_DIRECTORY_SIZE_BIT | (uint16_t)(flags & 0xFFFF));
        if (!(flags & MAP_EXECUTABLE)) set_table_nx((uint64_t*)(pml2v[pml2i]));

        releaseSpinlock(map->map_lock);
        return 0;
    }

    if (!(pml2v[pml2i] & PAGE_DIRECTORY_PRESENT))
    {
        struct physFrame new_frame = allocate_page_generic(6);
        pml2v[pml2i] = (uint64_t)(new_frame.phys_addr & ~PAGE_TABLE_ADDRESS_MASK);
        set_table_flags(&pml2v[pml2i], PAGE_DIRECTORY_PRESENT | PAGE_DIRECTORY_READWRITE | PAGE_DIRECTORY_USERSUPERVISOR);
    }

    uint64_t *pml1v = (uint64_t*)((pml2v[pml2i] & ~PAGE_TABLE_ADDRESS_MASK) + hhdm_response->offset);
    pml1v[pml1i] = ~PAGE_TABLE_ADDRESS_MASK & phys_addr.phys_addr;

    set_table_flags((uint64_t*)(pml1v[pml1i]), PAGE_DIRECTORY_SIZE_BIT | (uint16_t)(flags & 0xFFFF));
    if (!(flags & MAP_EXECUTABLE)) set_table_nx((uint64_t*)(pml1v[pml1i]));

    releaseSpinlock(map->map_lock);
    return 0;
}

#endif
