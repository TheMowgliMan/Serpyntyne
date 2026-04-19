#ifndef MM_PMM_H_
#define MM_PMM_H_

#include <stdint.h>
#include <stdbool.h>
#include <archutil/defines.h>

#define THREAD_ADDRESS_RANGE (PAGE_SIZE * (PAGE_SIZE / ARCH_POINTER_WIDTH))

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

inline uintptr_t __attribute__((force_inline)) phys_to_hhdm(uint64_t addr);
inline uint64_t __attribute__((force_inline)) hhdm_to_phys (uintptr_t hhdm_addr);
inline bool __attribute__((force_inline)) is_valid_frame(struct physFrame *f);

#endif
