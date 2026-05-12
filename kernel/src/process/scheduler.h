#ifndef PROC_SCHEDULER_H_
#define PROC_SCHEDULER_H_

#include <util/atomics.h>

#include <process/process_data.h>

#include <stdint.h>

#define IS_SCHEDULER_DONE(X) (X->ticks == 0xFFFF)

#define ITERATE_SCHEDULER_PROCESSES(X, Y) struct process *X = Y->phead; X != NULL; X = X->next

typedef struct scheduler {
    uint16_t ticks;
    spinlock_t *sched_lock;

    struct randomInstance *ri;
    struct process *phead;
    struct process *current;
} scheduler_t;

uint16_t scheduler_tick(scheduler_t *ctx);
void init_scheduler(scheduler_t *ctx);

struct process *scheduler_add_procedure(scheduler_t *ctx, int (*proc) (void), size_t stack_size);

extern scheduler_t *bsp_sched_ctx;

int test_proc1(void);
int test_proc2(void);

#endif
