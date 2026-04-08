#ifndef PANIC_H_
#define PANIC_H_

#include <archutil/intframe.h>

#define SPINLOCK_LONG_HOLD 1

void panic(struct intframe* iframe);
void exception(uint64_t reason, int64_t data1, int64_t data2);

#endif
