#ifndef PROC_SCHEDULER_H_
#define PROC_SCHEDULER_H_

#include <util/atomics.h>
#include <stdint.h>

typedef struct scheduler {
    uint16 ticks;
    spinlock_t sched_lock;
} scheduler_t;

#endif
