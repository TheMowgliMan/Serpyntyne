#include <process/scheduler.h>

#include <util/atomics.h>
#include <util/random.h>

#include <archutil/asmstubs.h>

#include <archprocess/task_switch.h>

#include <mm/heap.h>
#include <mm/vmm.h>

#include <acpi/acpi.h>

#include <terminal.h>

#include <stdint.h>

scheduler_t *bsp_sched_ctx;

uint32_t pid_count;
spinlock_t pid_count_lock;

void init_scheduler(scheduler_t *ctx)
{
    //kprintf("%x\r\n", (int64_t)ctx);

    ctx->ticks = 1000;

    ctx->sched_lock = (spinlock_t*)kvmalloc(sizeof(spinlock_t));
    initSpinlock(ctx->sched_lock);

    ctx->ri = (struct randomInstance*)kvmalloc(sizeof(struct randomInstance));
    init_rand_instance(ctx->ri);

    ctx->phead = NULL;
    ctx->current = NULL;

    pid_count = 0;
    initSpinlock(&pid_count_lock);
}

static struct process *find_highest_priority(scheduler_t *ctx)
{
    struct process *ret = NULL;
    uint32_t last_prio = 0;
    for (ITERATE_SCHEDULER_PROCESSES(cur, ctx))
    {
        if (cur == NULL)
        {
            //kprintf("Broke loop\r\n");
            break;
        }

        //kprintf("Process addr %x\r\n", (int64_t)cur);

        uint32_t cur_prio = get_priority(cur);
        //kprintf("PID %d priority: %d\r\n", (int64_t)cur->pid, (int64_t)cur_prio);
        if (cur_prio > last_prio || ret == NULL)
        {
            ret = cur;
            last_prio = cur_prio;
        }
    }

    //for (;;) { ; }

    return ret;
}

static void reschedule(scheduler_t *ctx)
{
    jailbreak_terminal();
    struct process *next_task = NULL;
    if (ctx->current != NULL)
        next_task = ctx->current->next;
    if (next_task == NULL)
        next_task = ctx->phead;

    if (next_task != ctx->current)
        kprintf("Next task: %d\r\n", (int64_t)next_task->pid);

    uint32_t prio = get_priority(next_task);
    int64_t slices = 500;
    // if (next_task->real_time_priority + 3 > prio)
    //     slices = next_task->real_time_priority + 3;
    // else
    //     slices = randrange_instance(ctx->ri, next_task->real_time_priority + 3, (prio > 51) ? 51 : prio);

    ctx->ticks = (uint16_t)slices;

    if (ctx->current != NULL)
        ctx->current->is_cur = 0;

    struct process *previous_task = ctx->current;

    if (previous_task != NULL)
        previous_task->dynamic_priority = 0;

    ctx->current = next_task;
    ctx->current->is_cur = 1;

    releaseSpinlock(ctx->sched_lock);
    switch_task(previous_task, next_task);
    //kprintf("%d\r\n", sizeof(struct process));
}

uint16_t scheduler_tick(scheduler_t *ctx)
{
    acquireSpinlock(ctx->sched_lock, 0);

    if (ctx->ticks == 0)
        reschedule(ctx);

    ctx->ticks -= 1;
    uint16_t ret = ctx->ticks;

    for (ITERATE_SCHEDULER_PROCESSES(cur, ctx))
    {
        if (cur == NULL)
            break;

        if (cur->is_cur == 0)
        {
            cur->ticks_since_last_run++;
            cur->dynamic_priority += (randrange_instance(ctx->ri, 0, 60) == 0) ? 1 : 0;
        }
    }

    releaseSpinlock(ctx->sched_lock);
    return ret;
}

static struct process *create_process(scheduler_t *ctx, uint32_t pid)
{
    struct process *p = (struct process*)kvcalloc(sizeof(struct process));

    kprintf("Created process @ %x\r\n", (int64_t)p);

    p->pid = pid;
    p->locked = PROC_LOCK_UNLOCKED;
    p->lock_target = 0;
    p->proc_map = NULL;
    p->static_priority = 1;
    p->dynamic_priority = 0;
    p->real_time_priority = PROC_RT_NONE;
    p->ticks_since_last_run = 0;
    p->desired_rt_interval = 1;
#ifdef __x86_64__
    p->rsp = 0;
#endif
    p->is_cur = false;
    p->next = NULL;

    if (ctx->phead == NULL)
        ctx->phead = p;
    else
    {
        for (ITERATE_SCHEDULER_PROCESSES(cur, ctx))
        {
            if (cur->next == NULL)
            {
                cur->next = p;
                break;
            }
        }
    }

    return p;
}

int test_proc1(void)
{
    while (1)
    {
        for (uint64_t i = 0; i < 50000; i++)
            pause();
        kputs_unlocked("|");
    }

    return 0;
}

int test_proc2(void)
{
    while (1)
    {
        for (uint64_t i = 0; i < 50000; i++)
            pause();
        kputs_unlocked("-");
    }

    return 0;
}

uint32_t get_new_pid(void)
{
    acquireSpinlock(&pid_count_lock, 0);
    uint32_t ret = pid_count;
    pid_count++;
    releaseSpinlock(&pid_count_lock);
    return ret;
}

void prepare_task(void)
{
    APIC_EOI();
    STI();
}

struct process *scheduler_add_procedure(
                             scheduler_t *ctx,
                             int (*proc) (void),
                             size_t stack_size
)
{
    acquireSpinlock(ctx->sched_lock, 0);

    struct process *p = create_process(ctx, get_new_pid());
    p->proc_map = kernel_page_table;

    uint64_t *stack = (uint64_t*)kvcalloc(stack_size);
    stack[(stack_size / 8) - 1] = (uint64_t)proc;
    stack[(stack_size / 8) - 2] = (uint64_t)(&prepare_task);
    p->rsp = (uint64_t)(stack + (stack_size / 8) - 8);

    releaseSpinlock(ctx->sched_lock);

    return p;
}
