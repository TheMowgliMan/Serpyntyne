#include <mm/pmm.h>
#include <limine.h>
#include <terminal.h>
#include <util/panic.h>
#include <stdint.h>
#include <util/atomics.h>
#include <util/random.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};

struct limine_memmap_response *memmap_response;

struct pmmFreePageSllNode* low_frames; // For below 4GiB allocations
struct pmmFreePageSllNode* small_high_frames; // For above 4GiB allocations, in PAGE_SIZE chunks
struct pmmFreePageSllNode* large_high_frames; // For above 4GiB allocations, in LARGE_PAGE_SIZE chunks

spinlock_t lfl;
spinlock_t shfl;
spinlock_t lhfl;

spinlock_t *low_frame_lock = &lfl;
spinlock_t *small_high_frame_lock = &shfl;
spinlock_t *large_high_frame_lock = &lhfl;

uint64_t initPmm(void)
{
    if (memmap_request.response == NULL)
    {
        exception(NO_MEMORY, 0, 0);
    }

    memmap_response = memmap_request.response;

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
            kprintf("Usable RAM: base %x, length %x\r\n",
                    (int64_t)(entry->base),
                    (int64_t)(entry->length));
#endif
        }
    }

    return ram;
}
