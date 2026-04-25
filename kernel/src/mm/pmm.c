#include <mm/pmm.h>
#include <util/liminereq.h>
#include <terminal.h>
#include <util/panic.h>
#include <stdint.h>
#include <util/atomics.h>
#include <util/random.h>
#include <util/align.h>
#include <archutil/defines.h>
#include <stdbool.h>
#include <memory.h>

/*
 * I call this doohickey a "curtain allocator," but somebody probably already has a name for this so please tell me if so.
 * The "curtain" is the whole allocator.
 * The "frame" is an array of pointers to the threads.
 * The "threads" are singly linked lists of beads (this is where the spinlocks are).
 * The "beads" are individual linked list nodes.
 */

struct pmmFreePageSllNode **frame = NULL;
spinlock_t *thread_locks;
uint64_t frame_len = 0;

struct randomInstance pmm_randomness;

void fill_free_lists(uint64_t ram)
{
    /*
     * To store the threads of the curtain, we need to allocate some pages via hhdm.
     * We need pages_to_allocate of each.
     */
    uint64_t pages_to_allocate = ALIGN_UP(ram, 0x40000000ull) / 0x40000000ull; // Divide and round up
    frame_len = pages_to_allocate * (PAGE_SIZE / ARCH_POINTER_WIDTH);

    // This is used to temporarily store pages before the "curtain" is "hanged".
    struct pmmFreePageSllNode *pages = NULL;

    uint64_t pages_added = 0; // For statistical purposes

    // Now we fill the "pages" list.
    for (uint64_t fill_entry = 0; fill_entry < memmap_response->entry_count; fill_entry++)
    {
        uint64_t entry_offset = 0;

        struct limine_memmap_entry *entry_struct = memmap_response->entries[fill_entry];

        if (entry_struct->type != LIMINE_MEMMAP_USABLE) continue;

        uint64_t start = ALIGN_UP(entry_struct->base, PAGE_SIZE);
        uint64_t end   = ALIGN_DOWN(entry_struct->base + entry_struct->length, PAGE_SIZE);

        if (start < MINIMUM_MEM_BOUNDARY) start = MINIMUM_MEM_BOUNDARY;

        if (start >= end) continue;

        if (entry_offset == 0) entry_offset = start - entry_struct->base;

        while (entry_struct->base + entry_offset < end)
        {
            uint64_t addr = entry_struct->base + entry_offset;
            struct pmmFreePageSllNode *node = (struct pmmFreePageSllNode*)(addr + hhdm_response->offset);

            if (addr + PAGE_SIZE < end && node != NULL)
            {
                node->size = 0;
                node->next = pages;
                pages = node;

                pages_added++;
                entry_offset += PAGE_SIZE;
            }
            else
            {
                break;
            }
        }
    }

#ifdef DEBUG
    klog(LOG_NOTICE, "Frames allocated: %d\r\n",
            (int64_t)pages_added);
#endif
    /*
     * Now we have to allocate two sets of pages_to_allocate contiguous pages.
     * As there's hardly anything in memory, however, this should be easy to fulfill.
     */
    uint64_t pages_removed = 0;
    struct pmmFreePageSllNode *cur;
    for (uint32_t i = 0; i < pages_to_allocate; i++)
    {
        cur = pages;
        if ((uint64_t)(cur) - (uint64_t)(cur->next) != PAGE_SIZE)
        {
            i = 0;
        }

        pages = cur->next;

        if (pages == NULL || pages == 0) exception(OUT_OF_MEMORY, OOM_PMM_FRAME_CREATION, 0);
        pages_removed++;
    }

#ifdef DEBUG
    klog(LOG_NOTICE, "Creating frame...\r\n");
#endif

    frame = (struct pmmFreePageSllNode**)(cur);
    memset(frame, 0, pages_to_allocate * PAGE_SIZE);

    /* Now for the spinlocks. */
    for (uint64_t i = 0; i < pages_to_allocate; i++)
    {
        cur = pages;
        if ((uint64_t)(cur) - (uint64_t)(cur->next) != PAGE_SIZE)
        {
            i = 0;
        }

        pages = cur->next;

        if (pages == NULL || pages == 0) exception(OUT_OF_MEMORY, OOM_PMM_SPINLOCKS_CREATION, 0);
        pages_removed++;
    }

#ifdef DEBUG
    klog(LOG_NOTICE, "Pages lost: %d\r\n", pages_removed);
#endif

    /* The spinlocks also have to be initialized. */
    thread_locks = (spinlock_t*)(cur);
    for (uint64_t i = 0; i < (pages_to_allocate * 4096 / LOCK_SIZE); i++)
    {
        initSpinlock(&thread_locks[i]);
    }

#ifdef DEBUG
    klog(LOG_SUCCESS, "Spinlocks initialized! Filling PMM...\r\n");
#endif

    /*
     * Now that we've created a frame-array, we populate it from the freelist!
     * Nothing has to be done for the spinlocks here.
     */

    uint64_t i = 0;
    for (
        struct pmmFreePageSllNode *pop_frame = pages;
        1;
    )
    {
        i++;

        if (pop_frame == NULL)
            break;

        uint64_t idx = ((uint64_t)pop_frame - hhdm_response->offset) / THREAD_ADDRESS_RANGE;

        if (frame[idx] == 0 || frame[idx] == NULL)
        {

            uintptr_t tmp = (uintptr_t)(pop_frame->next);

            frame[idx] = pop_frame;
            frame[idx]->next = NULL;

            pop_frame = (struct pmmFreePageSllNode*)tmp;
        }
        else
        {
            uintptr_t tmp = (uintptr_t)(pop_frame->next);
            pop_frame->next = frame[idx];
            frame[idx] = pop_frame;

            pop_frame = (struct pmmFreePageSllNode*)tmp;
        }
    }

    /*
     * And now we should be all done!
     * Now that the pages have been added to the threads and the spinlocks initialized,
     * there is not much left to do. We will initialize pmm_randomness now.
     */

#ifdef DEBUG
    klog(LOG_SUCCESS, "Finished filling curtain allocator!\r\n\t" TTY_MAGENTA "Allocated %d pages.\r\n" TTY_RESET, (int64_t)i);
#endif

    init_rand_instance(&pmm_randomness);
}

struct physFrame allocate_page_generic(uint64_t thread_denoter)
{
    for (uint64_t i = 0; i < frame_len; i++)
    {
        uint64_t grab = (i + thread_denoter) % frame_len;

        acquireSpinlock(&thread_locks[grab], 0);

        if (frame[grab] != 0 && frame[grab] != NULL)
        {
            struct pmmFreePageSllNode *node = frame[grab];
            frame[grab] = node->next;

            struct physFrame ret;
            ret.phys_addr = (uintptr_t)node - (uintptr_t)(hhdm_response->offset);
            ret.size = 0;
            ret.is_low = false; // Even if it's below 4GiB, for all intents and purposes it isn't

            memset(node, 0, PAGE_SIZE);
            releaseSpinlock(&thread_locks[grab]);

            return ret;
        }

        releaseSpinlock(&thread_locks[grab]);
    }

    struct physFrame ret;
    ret.phys_addr = 0;
    return ret;
}

struct physFrame allocate_page_random(struct randomInstance *ri)
{
    struct randomInstance *used;
    if (ri == NULL)
        used = &pmm_randomness;
    else
        used = ri;

    rdtsc_seed_rand_instance(used);
    uint64_t thread_denoter = randrange_instance(used, frame_len / 2, frame_len);

    for (uint64_t i = 0; i < frame_len; i++)
    {
        uint64_t grab = (i + thread_denoter) % frame_len;

        acquireSpinlock(&thread_locks[grab], 0);

        if (frame[grab] != 0 && frame[grab] != NULL)
        {
            struct pmmFreePageSllNode *node = frame[grab];
            frame[grab] = node->next;

            struct physFrame ret;
            ret.phys_addr = (uintptr_t)node - (uintptr_t)(hhdm_response->offset);
            ret.size = 0;
            ret.is_low = false; // Even if it's below 4GiB, for all intents and purposes it isn't

            memset(node, 0, PAGE_SIZE);
            releaseSpinlock(&thread_locks[grab]);

            return ret;
        }

        releaseSpinlock(&thread_locks[grab]);
    }

    struct physFrame ret;
    ret.phys_addr = 0;
    return ret;
}

uint64_t ask_for_thread_denoter(void)
{
    return (uint64_t)randrange_instance(&pmm_randomness, frame_len / 2, frame_len);
}

bool should_get_new_thread_denoter(uint64_t td)
{
    if (frame[td] == 0 || frame[td] == NULL) return true; else return false;

     return false; // Shut up compiler
}

struct physFrame gen_frame(uintptr_t phys_addr, uint8_t size, bool is_low)
{
    struct physFrame ret;
    ret.phys_addr = phys_addr;
    ret.size = size;
    ret.is_low = is_low;
    return ret;
}

uint64_t initPmm(void)
{
    klog(LOG_PROC, "Starting physical memory manager...\r\n");

    uint64_t usable_ram = 0;
    uint64_t all_ram;
    for (uint64_t i = 0; i < memmap_response->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap_response->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE)
        {
            usable_ram += entry->length;
        }

        #ifdef DEBUG
        kprintf(TTY_MAGENTA "RAM found: " TTY_HICYAN "type %d, " TTY_BLUE "base %x, " TTY_HIMAGENTA "length %x\r\n" TTY_RESET,
                (int64_t)(entry->type),
                (int64_t)(entry->base),
                (int64_t)(entry->length));
        #endif

        if (memmap_response->entry_count - i != 1 || entry->type == LIMINE_MEMMAP_USABLE)
        {
            all_ram += entry->length;
        }
    }

    klog(LOG_NOTICE, TTY_HICYAN "Total of all ram: %d bytes\r\n" TTY_RESET, (int64_t)all_ram);

    fill_free_lists(all_ram);

    return usable_ram;
}
