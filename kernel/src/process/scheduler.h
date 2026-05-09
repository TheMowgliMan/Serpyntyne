#ifndef PROC_SCHEDULER_H_
#define PROC_SCHEDULER_H_

#include <util/atomics.h>

#include <process/process_data.h>

#include <stdint.h>

#define IS_SCHEDULER_DONE(X) (X->ticks == 0xFFFF)

#define ITERATE_SCHEDULER_PROCESSES(X, Y) struct process X = Y->phead; X != NULL; X = X->next

typedef struct scheduler {
    uint16 ticks;
    spinlock_t sched_lock;

    struct randomInstance *ri;
    struct process *phead;
} scheduler_t;

#endif
