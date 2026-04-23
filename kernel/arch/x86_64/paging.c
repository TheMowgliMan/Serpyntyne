#include <paging.h>
#include <stdint.h>

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

struct pagemap generate_pagemap(uint64_t *top_level, spinlock_t *spinlock)
{
    struct pagemap pm;
    pm.top_level = top_level;
    pm.map_lock = spinlock;
    return pm;
}
