#ifndef MM_PMM_H_
#define MM_PMM_H_

#include <stdint.h>
#include <stdbool.h>
#include <archutil/defines.h>
#include <util/random.h>
#include <util/liminereq.h>

#define THREAD_ADDRESS_RANGE TAR

struct pmmFreePageSllNode {
    struct pmmFreePageSllNode *next;
    uint8_t size; // The size in bytes is 2 ^ <size> * PAGE_SIZE (from archutil/defines.h)
};

struct physFrame {
    uintptr_t phys_addr;
    uint8_t size; // The size in bytes is 2 ^ <size> * PAGE_SIZE (from archutil/defines.h)
    bool is_low;
};

uint64_t initPmm(void);

struct physFrame allocate_page_generic(uint64_t thread_denoter);
struct physFrame allocate_page_random(struct randomInstance *ri);

uint64_t ask_for_thread_denoter(void);
bool should_get_new_thread_denoter(uint64_t td);

struct physFrame gen_frame(uintptr_t phys_addr, uint8_t size, bool is_low);

inline uintptr_t __attribute__((always_inline)) phys_to_hhdm(uint64_t addr)
{
    return (uintptr_t)(addr + hhdm_response->offset);
}

inline uint64_t __attribute__((always_inline)) hhdm_to_phys(uintptr_t hhdm_addr)
{
    return (uint64_t)(hhdm_addr - hhdm_response->offset);
}

inline bool __attribute__((always_inline)) is_valid_frame(struct physFrame *f)
{
    if (f->phys_addr == 0) return false; else return true;

    return false; // Shut up compiler
}

#endif
