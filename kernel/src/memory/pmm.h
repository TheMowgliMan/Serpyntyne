#ifndef PMM_H_
#define PMM_H_

struct pmmFreePageSllNode {
    pmmFreeSll *next;
}

struct pmmFreePageSllNode* low_frames[2]; // For below 4GiB allocations
struct pmmFreePageSllNode* high_frames[16]; // For above 4GiB allocations

#endif
