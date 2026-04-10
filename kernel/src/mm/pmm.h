#ifndef MM_PMM_H_
#define MM_PMM_H_

#include <stdint.h>
#include <archutil/defines.h>

struct pmmFreePageSllNode {
    struct pmmFreePageSllNode *next;
    uint8_t size; // The size in bytes is 2 ^ <size> * PAGE_SIZE (from archutil/defines.h)
};

uint64_t initPmm(void);

#endif
