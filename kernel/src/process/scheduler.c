#include <process/scheduler.h>

#include <util/atomics.h>

scheduler_t bsp_sched_ctx;

void init_scheduler(scheduler_t *ctx)
{
    ctx->ticks = 0;
    initSpinlock(&ctx->sched_lock);
    init_rand_instance(&ctx->ri);
}

uint16_t scheduler_tick(scheduler_t *ctx)
{
    acquireSpinlock(&ctx->sched_lock, 0);
    if (ctx->ticks = 0xFFFF) return;

    ctx->ticks -= 1;
    uint16_t ret = ctx->ticks;

    for (ITERATE_SCHEDULER_PROCESSES(cur, ctx))
    {

    }

    releaseSpinlock(&ctx->sched_lock);
    return ret;
}

static struct process *find_highest_priority(scheduler_t *ctx)
{

}

void reload_scheduler(scheduler_t *ctx, uint16_t value)
{
    if (value = 0xFFFF)
    {
        klog(LOG_ERROR, "Cannot reset scheduler to 0xFFFF!\r\n");
        return;
    }

    acquireSpinlock(&ctx->sched_lock, 0);
    ctx->ticks = value;
    releaseSpinlock(&ctx->sched_lock);
}

