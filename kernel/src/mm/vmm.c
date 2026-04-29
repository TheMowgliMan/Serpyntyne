#include <mm/vmm.h>
#include <mm/pmm.h>
#include <terminal.h>
#include <stdint.h>
#include <stdbool.h>
#include <paging.h>
#include <archutil/defines.h>
#include <util/liminereq.h>
#include <util/atomics.h>
#include <util/align.h>
#include <util/panic.h>
#include <util/elf.h>
#include <memory.h>

struct pagemap kpt;
struct pagemap *kernel_page_table;

spinlock_t kernel_vmm_lock;

void debug_hhdm(uintptr_t vaddr, uintptr_t paddr, uint64_t flags, bool isbig)
{
    // klog(LOG_NOTICE, "Mapping HHDM: %x flags, %x vaddr, %x paddr, ", (int64_t)flags, (int64_t)vaddr, (int64_t)paddr);
    // if (isbig)
    //     kprintf("big: yes\r\n");
    // else
    //     kprintf("big: no\r\n");
}

void paging_init(void)
{
    klog(LOG_PROC, "Starting Virtual Memory Manager...\r\n");

    initSpinlock(&kernel_vmm_lock);

    struct physFrame tmpf = allocate_page_random(NULL);
    uint64_t *kernel_top_level_paging_directory = (uint64_t *)(tmpf.phys_addr + hhdm_response->offset);

    uintptr_t kernel_top_level_paging_directory_phys_addr = tmpf.phys_addr;

    kpt = generate_pagemap(kernel_top_level_paging_directory, &kernel_vmm_lock);
    kernel_page_table = &kpt;

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

        uintptr_t floor = (uintptr_t)(ALIGN_DOWN(entry->base, PAGE_SIZE));
        uintptr_t cieling = (uintptr_t)(ALIGN_UP(entry->base + entry->length, PAGE_SIZE));

        uint64_t flags = MAP_READABLE | MAP_WRITABLE | MAP_EXECUTABLE | MAP_USER;
        if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER)
            flags |= MAP_WRITETHROUGH | MAP_CACHEDISABLE;

        uintptr_t addr = floor;


        while (addr < cieling)
        {
            if (addr + LARGE_PAGE_SIZE <= cieling && IS_ALIGNED(addr, LARGE_PAGE_SIZE))
            {
                debug_hhdm((uintptr_t)(addr + hhdm_response->offset), addr, flags, true);
                tmpf = gen_frame(addr, LARGE_PAGE_SIZE_EXPONENT, false);

                map_page(kernel_page_table, (uintptr_t)(addr + hhdm_response->offset), tmpf, flags);
                addr += LARGE_PAGE_SIZE;
            }
            else if (addr + PAGE_SIZE <= cieling && IS_ALIGNED(addr, PAGE_SIZE))
            {
                debug_hhdm((uintptr_t)(addr + hhdm_response->offset), addr, flags, false);
                tmpf = gen_frame(addr, PAGE_SIZE_EXPONENT, false);

                map_page(kernel_page_table, (uintptr_t)(addr + hhdm_response->offset), tmpf, flags);
                addr += PAGE_SIZE;
            }
        }
    }

#ifdef DEBUG
    klog(LOG_INFO, "Mapping kernel...\r\n");
#endif

    void *kexecf_addr = executable_file_response->executable_file->address;
    struct elf_header_64 *kernel_header = (struct elf_header_64*)kexecf_addr;
    uint8_t *arr = (uint8_t*)kexecf_addr;
    struct elf_program_header_64 *kernel_program_headers = (struct elf_program_header_64*)(arr + kernel_header->program_header_table_offset);

    char elf_magic[4] = {0x7f, 0x45, 0x4c, 0x46};
    if (memcmp(kernel_header->magic, elf_magic, 4) != 0)
    {
        klog(LOG_ERROR, "Bad kernel elf magic value! Expected %x, got %x!", (int64_t)(ELF_MAGIC), (int64_t)(kernel_header->magic));
        for (;;) { ; }
    }

    klog(LOG_NOTICE, "Header count: %d\r\n", (int64_t)kernel_header->program_header_entry_count);

    for (uint64_t i = 0; i < kernel_header->program_header_entry_count; i++)
    {
        struct elf_program_header_64 *cur_h = &kernel_program_headers[i];
        if (cur_h->type_of_segment != ELF_PHEADER_SEGMENT_TYPE_LOAD)
        {
            klog(LOG_ERROR, "Non-load segment, was segment %d!\r\n", (int64_t)i);
            continue;
        }

        uint64_t flags = MAP_READABLE | MAP_KERNEL | MAP_NEWMAP;

        if (cur_h->flags & ELF_PHEADER_FLAGS_EXECUTABLE)
            flags |= MAP_EXECUTABLE;
        if (cur_h->flags & ELF_PHEADER_FLAGS_WRITABLE)
            flags |= MAP_WRITABLE;

        uint64_t pages_to_map = ALIGN_UP(cur_h->p_memsize, PAGE_SIZE) / PAGE_SIZE;
        uintptr_t virtual_base = cur_h->p_vaddr + executable_address_response->virtual_base - KERNEL_ENTRY_POINT;
        uintptr_t physical_base = executable_address_response->physical_base + cur_h->p_vaddr - KERNEL_ENTRY_POINT;

        klog(LOG_NOTICE, "Kernel map: flags %x, pages %d, virtual_base %x, physical_base %x\r\n", (int64_t)flags, (int64_t)pages_to_map, (int64_t)virtual_base, (int64_t)physical_base);

        for (uint64_t page = 0; page < pages_to_map; page++)
        {
            uintptr_t v_addr = virtual_base + (page * PAGE_SIZE);
            uintptr_t p_tmp = physical_base + (page * PAGE_SIZE);
            struct physFrame p_addr = gen_frame(p_tmp, PAGE_SIZE_EXPONENT, false);

            map_page(kernel_page_table, v_addr, p_addr, flags);
        }
    }

#ifdef DEBUG
    klog(LOG_NOTICE, "Switching to new page table!\r\n");
#endif

    uintptr_t kptaddr = kernel_top_level_paging_directory_phys_addr;
    load_cr3(kptaddr);

    klog(LOG_SUCCESS, "Virtual Memory Manager started!\r\n");
}

int map_multiple_pages(struct pagemap *map, uint64_t pages, uintptr_t virt_addr, struct physFrame phys_addr, uint64_t flags)
{
    check_page_align(virt_addr, phys_addr.phys_addr, PAGE_SIZE);
    uint8_t size = phys_addr.size;

    struct physFrame phys = gen_frame(phys_addr.phys_addr, size, false);

    uint64_t inc = PAGE_SIZE;
    if (size == LARGE_PAGE_SIZE_EXPONENT)
        inc = LARGE_PAGE_SIZE;

    uint64_t page;
    for (page = 0; page < pages; page++)
    {
        map_page(map, virt_addr + (inc * page), phys, flags);
        phys.phys_addr += inc;
    }

    return (int)page;
}

void *allocate_random_and_map(struct pagemap *map,
                              struct randomInstance *ri,
                              size_t bytes,
                              uintptr_t virt_addr,
                              uint64_t flags)
{
    size_t pages_to_allocate = ALIGN_UP(bytes, PAGE_SIZE) / PAGE_SIZE;
    uintptr_t virt_base = ALIGN_DOWN(virt_addr, PAGE_SIZE);

    for (uint64_t page = 0; page < pages_to_allocate; page++)
    {
        struct physFrame phys_addr = allocate_page_random(ri);
        map_page(map, virt_base + (page * PAGE_SIZE), phys_addr, flags);
    }

    return (void*)virt_addr;
}
