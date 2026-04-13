#include <mm/pmm.h>
#include <limine.h>
#include <terminal.h>
#include <util/panic.h>
#include <stdint.h>
#include <util/atomics.h>
#include <util/random.h>
#include <util/align.h>
#include <archutil/defines.h>
#include <stdbool.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

struct limine_memmap_response *memmap_response;
struct limine_hhdm_response *hhdm_response;

struct pmmFreePageSllNode* low_frames; // For below 4GiB allocations
struct pmmFreePageSllNode* small_high_frames; // For above 4GiB allocations, in PAGE_SIZE chunks
struct pmmFreePageSllNode* large_high_frames; // For above 4GiB allocations, in LARGE_PAGE_SIZE chunks

spinlock_t lfl;
spinlock_t shfl;
spinlock_t lhfl;

spinlock_t *low_frame_lock = &lfl;
spinlock_t *small_high_frame_lock = &shfl;
spinlock_t *large_high_frame_lock = &lhfl;

void fill_free_lists(void)
{
    acquireSpinlock(low_frame_lock, 0);
    acquireSpinlock(small_high_frame_lock, 0);
    acquireSpinlock(large_high_frame_lock, 0);

    low_frames = NULL;
    small_high_frames = NULL;
    large_high_frames = NULL;

    uint32_t low_frames_added = 0;
    uint32_t small_high_frames_added = 0;
    uint32_t large_high_frames_added = 0;

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

            if (addr + PAGE_SIZE < LOW_MEM_BOUNDARY)
            {
                node->size = 0;
                node->next = low_frames;
                low_frames = node;

                low_frames_added++;
                entry_offset += PAGE_SIZE;
            }
            else if (addr + LARGE_PAGE_SIZE < end && randrange(0, CREATE_LARGE_PAGE_CHANCE) == 0) // Feels like OSDev Discord ragebait right here
            {
                node->size = LARGE_PAGE_SIZE_EXPONENT;
                node->next = large_high_frames;
                large_high_frames = node;

                large_high_frames_added++;
                entry_offset += LARGE_PAGE_SIZE;
            }
            else if (addr + PAGE_SIZE < end)
            {
                node->size = 0;
                node->next = small_high_frames;
                small_high_frames = node;

                small_high_frames_added++;
                entry_offset += PAGE_SIZE;
            }
            else
            {
                break;
            }
        }
    }

#ifdef DEBUG
    kprintf("Frames allocated:\r\nLow frames: %d\r\nSmall frames: %d\r\nLarge frames: %d\r\n",
            (int64_t)low_frames_added,
            (int64_t)small_high_frames_added,
            (int64_t)large_high_frames_added);
#endif

    releaseSpinlock(low_frame_lock);
    releaseSpinlock(small_high_frame_lock);
    releaseSpinlock(large_high_frame_lock);
}

uintptr_t phys_to_hhdm(uint64_t addr)
{
    return (uintptr_t)(addr + hhdm_response->offset);
}

uint64_t hhdm_to_phys(uintptr_t hhdm_addr)
{
    return (uint64_t)(hhdm_addr - hhdm_response->offset);
}

struct physFrame allocateFrame(frame_type_t type)
{
    struct pmmFreePageSllNode *node;
    struct physFrame ret;

    if (type == NORMAL_FRAME)
    {
        acquireSpinlock(small_high_frame_lock, 0);

        if (small_high_frames == NULL)
        {
            acquireSpinlock(low_frame_lock, 0);

            if (low_frames == NULL)
            {
                ret.phys_addr = 0;
                return ret;
            }

            node = low_frames;
            low_frames = node->next;
            releaseSpinlock(low_frame_lock);

            ret.is_low = true;
        }
        else
        {
            node = small_high_frames;
            small_high_frames = node->next;

            ret.is_low = false;
        }

        releaseSpinlock(small_high_frame_lock);
    }
    else if (type == LARGE_FRAME)
    {
        acquireSpinlock(large_high_frame_lock, 0);

        if (large_high_frames == NULL)
        {
            ret.phys_addr = 0;
            return ret;
        }

        node = large_high_frames;
        large_high_frames = node->next;
        releaseSpinlock(large_high_frame_lock);

        ret.is_low = false;
    }
    else if (type == LOW_FRAME)
    {
        acquireSpinlock(low_frame_lock, 0);

        if (low_frames == NULL)
        {
            ret.phys_addr = 0;
            return ret;
        }

        node = low_frames;
        low_frames = node->next;
        releaseSpinlock(low_frame_lock);

        ret.is_low = true;
    }

    ret.phys_addr = (uint64_t)((uintptr_t)node - hhdm_response->offset);
    ret.size = node->size;

    return ret;
}

void freeFrame(struct physFrame frame)
{
    if (frame.is_low)
    {
        if (frame.size != 0) exception(BAD_FRAME_SIZE, frame.size, (int64_t)frame.phys_addr);

        acquireSpinlock(low_frame_lock, 0);

        struct pmmFreePageSllNode *node = (struct pmmFreePageSllNode*)(frame.phys_addr + hhdm_response->offset);
        node->size = frame.size;

        node->next = low_frames;
        low_frames = node;
    }
    else if (frame.size != 0 && frame.size != LARGE_PAGE_SIZE_EXPONENT)
        exception(BAD_FRAME_SIZE, frame.size, (int64_t)frame.phys_addr);

    bool is_large = false;
    if (frame.size == LARGE_PAGE_SIZE_EXPONENT) is_large = true;

    if (is_large) acquireSpinlock(large_high_frame_lock, 0); else acquireSpinlock(small_high_frame_lock, 0);

    struct pmmFreePageSllNode *node = (struct pmmFreePageSllNode*)(frame.phys_addr + hhdm_response->offset);
    node->size = frame.size;

    if (is_large) node->next = large_high_frames; else node->next = small_high_frames;
    if (is_large) large_high_frames = node; else small_high_frames = node;

    if (is_large) releaseSpinlock(large_high_frame_lock); else releaseSpinlock(small_high_frame_lock);
}

uint64_t initPmm(void)
{
    if (memmap_request.response == NULL) exception(NO_MEMORY, 0, 0);

    if (hhdm_request.response == NULL) exception(NO_HHDM, 0, 0);

    memmap_response = memmap_request.response;
    hhdm_response = hhdm_request.response;

    initSpinlock(low_frame_lock);
    initSpinlock(small_high_frame_lock);
    initSpinlock(large_high_frame_lock);

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

    fill_free_lists();

    return ram;
}
