#ifndef MM_PMM_H_
#define MM_PMM_H_

#include <stdint.h>
#include <stdbool.h>
#include <archutil/defines.h>

#define CREATE_LARGE_PAGE_CHANCE 256 // One in <this + 1> chance

struct pmmFreePageSllNode {
    struct pmmFreePageSllNode *next;
    uint8_t size; // The size in bytes is 2 ^ <size> * PAGE_SIZE (from archutil/defines.h)
};

struct physFrame {
    uint64_t phys_addr;
    uint8_t size; // The size in bytes is 2 ^ <size> * PAGE_SIZE (from archutil/defines.h)
    bool is_low;
};

enum frame_type { NORMAL_FRAME, LARGE_FRAME, LOW_FRAME };
typedef enum frame_type frame_type_t;

uint64_t initPmm(void);

struct physFrame allocateFrame(frame_type_t type);
void freeFrame(struct physFrame frame);

#endif
