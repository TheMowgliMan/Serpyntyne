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
    uint64_t pages_to_allocate = (ram + 0x3FFFFFFFull) / 0x40000000ull // Divide and round up
    frame_len = pages_to_allocate * (PAGE_SIZE / ARCH_POINTER_WIDTH);

    // This is used to temporarily store pages before the "curtain" is "hanged".
    struct pmmFreePageSllNode *pages = NULL;

    uint32_t pages_added = 0; // For statistical purposes

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

            if (addr + PAGE_SIZE < end)
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
    kprintf("Frames allocated: %d\r\n",
            (int64_t)pages_added);
#endif
    /*
     * Now we have to allocate two sets of pages_to_allocate contiguous pages.
     * As there's hardly anything in memory, however, this should be easy to fulfill.
     */
    struct pmmFreePageSllNode *cur;
    for (uint32_t i = 0; i < pages_to_allocate; i++)
    {
        cur = pages;
        if ((uint64_t)(cur) - (uint64_t)(cur->next) != PAGE_SIZE)
        {
            i = 0;
        }

        pages = cur->next;

        if (pages == NULL || pages == 0) exception(OUT_OF_MEMORY, OOM_PMM_FRAME_CREATON, 0);
    }

    frame = (pmmFreePageSllNode**)(cur);
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

        if (pages == NULL || pages == 0) exception(OUT_OF_MEMORY, OOM_PMM_SPINLOCKS_CREATON, 0);
    }

    /* The spinlocks also have to be initialized. */
    thread_locks = (spinlock_t*)(cur);
    for (uint64_t i = 0; i < (pages_to_allocate * 4096) / LOCK_SIZE; i++)
    {
        initSpinlock(&thread_locks[i]);
    }

    /*
     * Now that we've created a frame-array, we populate it from the freelist!
     * Nothing has to be done for the spinlocks here.
     */
    for (
        struct pmmFreePageSllNode *pop_frame = pages;
        pop_frame != NULL;
        pop_frame = pop_frame->next;
    )
    {
        uint64_t idx = (uint64_t)pop_frame / THREAD_ADDRESS_RANGE;
        if (frame[idx] == 0 || frame[idx] == NULL)
        {
            frame[idx] = pop_frame;
            frame[idx]->next = NULL;
        }
        else
        {
            struct pmmFreePageSllNode *tmp = pop_frame;
            tmp->next = frame[idx];
            frame[idx] = tmp;
        }
    }

    /*
     * And now we should be all done!
     * Now that the pages have been added to the threads and the spinlocks initialized,
     * there is not much left to do. We will initialize pmm_randomness now.
     */

#ifdef DEBUG
    kprintf("Finished filling curtain allocator!\r\n");
#endif

    init_rand_instance(&pmm_randomness);
}

struct physFrame allocate_page_generic(uint64_t thread_denoter)
{
    for (uint64_t i = 0, i < frame_len, i++)
    {
        uint64_t grab = (i + thread_denoter) % frame_len;

        acquireSpinlock(&thread_locks[grab]);

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
    struct randomInstace *used;
    if (ri == NULL)
        used = &pmm_randomness;
    else
        used = ri;

    rdtsc_seed_rand_instance(used);
    uint64_t thread_denoter = randrange_instance(used, frame_len / 2, frame_len);

    for (uint64_t i = 0, i < frame_len, i++)
    {
        uint64_t grab = (i + thread_denoter) % frame_len;

        acquireSpinlock(&thread_locks[grab]);

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
    if (frame[grab] == 0 || frame[grab] == NULL) return true; else return false;
}

inline uintptr_t __attribute__((force_inline)) phys_to_hhdm(uint64_t addr)
{
    return (uintptr_t)(addr + hhdm_response->offset);
}

inline uint64_t __attribute__((force_inline)) hhdm_to_phys (uintptr_t hhdm_addr)
{
    return (uint64_t)(hhdm_addr - hhdm_response->offset);
}

inline bool __attribute__((force_inline)) is_valid_frame(struct physFrame *f)
{
    if (f->phys_addr == 0) return false; else return true;
}

uint64_t initPmm(void)
{
    uint64_t ram = 0;
    for (uint64_t i = 0; i < memmap_response->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap_response->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE)
        {
            ram += entry->length;
#ifdef DEBUG
            kprintf("Usable RAM found: base %x, length %x\r\n",
                    (int64_t)(entry->base),
                    (int64_t)(entry->length));
#endif
        }
    }

    fill_free_lists(ram);

    return ram;
}
