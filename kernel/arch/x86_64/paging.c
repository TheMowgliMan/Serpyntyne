#include <paging.h>
#include <stdint.h>
#include <util/align.h>

void check_page_align(uintptr_t virt_addr, uintptr_t phys_addr, uint64_t align)
{
    if (!IS_ALIGNED(virt_addr, align))
    {
        exception(MISALIGNED_PAGE, (int64_t)virt_addr, (int64_t)phys_addr);
    }

    if (!IS_ALIGNED(phys_addr, align))
    {
        exception(MISALIGNED_PAGE, (int64_t)virt_addr, (int64_t)phys_addr);
    }
}

inline static uint64_t page_cycle(uint64_t table, uint64_t flags)
{
    //uint64_t addr = table + hhdm_response->offset;
    uint64_t *page_table_entry = (uint64_t*)(table + hhdm_response->offset);
    uint64_t entry_value = *page_table_entry;

    if (!(entry_value & PAGE_FLAG_PRESENT))
    {
        struct physFrame new_table = allocate_page_random(NULL);
        uint64_t new_table_int = new_table.phys_addr;

        entry_value = new_table_int | flags;
        *page_table_entry = entry_value;
    }

    return entry_value & ~PAGE_TABLE_ADDRESS_MASK;
}

int map_page(struct pagemap *map, uintptr_t virt_addr, struct physFrame phys_addr, uint64_t flags)
{
    check_page_align(virt_addr, phys_addr.phys_addr, PAGE_SIZE);

    if (phys_addr.size == LARGE_PAGE_SIZE_EXPONENT)
        check_page_align(virt_addr, phys_addr.phys_addr, LARGE_PAGE_SIZE);

    if (!(((1ull << phys_addr.size) * PAGE_SIZE) & ACCEPTABLE_PAGE_SIZES))
    {
        kprintf("did this\r\n");
        exception(ILLEGAL_PAGE_MAP_SIZE, (int64_t)phys_addr.size, (int64_t)virt_addr);
    }

    acquireSpinlock(map->map_lock, 0);

    // Thanks EvalynOS :P
    uint16_t pml1i = (virt_addr >> 12) & 0x1ff;
    uint16_t pml2i = (virt_addr >> 21) & 0x1ff;
    uint16_t pml3i = (virt_addr >> 30) & 0x1ff;
    uint16_t pml4i = (virt_addr >> 39) & 0x1ff;

    uint64_t cur_pte = (uint64_t)(map->top_level) - hhdm_response->offset;
    uint64_t cur_pte_phys = cur_pte + pml4i * ARCH_DATA_WIDTH;
    cur_pte = page_cycle(cur_pte_phys, NEW_PAGE_TABLE_FLAGS);

    cur_pte_phys = cur_pte + pml3i * ARCH_DATA_WIDTH;
    cur_pte = page_cycle(cur_pte_phys, NEW_PAGE_TABLE_FLAGS);

    cur_pte_phys = cur_pte + pml2i * ARCH_DATA_WIDTH;

    if (phys_addr.size == LARGE_PAGE_SIZE_EXPONENT)
    {
        uint64_t *cur_pte_virt = (uint64_t*)(cur_pte_phys + hhdm_response->offset);
        *cur_pte_virt = (phys_addr.phys_addr & ~PAGE_TABLE_ADDRESS_MASK) | flags | PAGE_DIRECTORY_SIZE_BIT;

        asm volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");

        releaseSpinlock(map->map_lock);
        return 0;
    }


    cur_pte = page_cycle(cur_pte_phys, NEW_PAGE_TABLE_FLAGS);
    uint64_t *cur_pte_virt = (uint64_t*)(cur_pte + pml1i * ARCH_DATA_WIDTH + hhdm_response->offset);
    *cur_pte_virt = (phys_addr.phys_addr & ~PAGE_TABLE_ADDRESS_MASK) | flags;

    asm volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");

    releaseSpinlock(map->map_lock);
    return 0;
}

struct pagemap generate_pagemap(uint64_t *top_level, spinlock_t *spinlock)
{
    struct pagemap pm;
    pm.top_level = top_level;
    pm.map_lock = spinlock;

    pm.kernel_start = 0;
    pm.kernel_end = 0;

    pm.fs_end = 0;
    pm.gs_end = 0;

    pm.heap_start = 0;
    pm.heap_offset = 0;

    return pm;
}
