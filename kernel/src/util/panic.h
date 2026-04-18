#ifndef PANIC_H_
#define PANIC_H_

#include <archutil/intframe.h>

#define SPINLOCK_LONG_HOLD 1
#define NO_MEMORY 2
#define NO_HHDM 3
#define BAD_FRAME_SIZE 4
#define ILLEGAL_PAGE_MAP_SIZE 5
#define OUT_OF_MEMORY 6 // NOT the same as NO_MEMORY

#define OOM_PMM_FRAME_CREATION 0ll
#define OOM_PMM_SPINLOCKS_CREATION 1ll

void panic(struct intframe* iframe);
void exception(uint64_t reason, int64_t data1, int64_t data2);

#endif
