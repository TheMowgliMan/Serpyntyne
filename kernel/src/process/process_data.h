#ifndef PROCESS_PROCESS_DATA_H_
#define PROCESS_PROCESS_DATA_H_

#include <util/liminereq.h>

#include <stdint.h>

#define PROC_LOCK_UNLOCKED 0
#define PROC_LOCK_PROCESS 1
#define PROC_LOCK_SLEEPING 0xFFFFFFFF

#define PROC_RT_AVOID (1 << 0)
#define PROC_RT_NONE (1 << 1)
#define PROC_RT_RECOMMEND (1 << 2)
#define PROC_RT_IMPORTANT (1 << 3)
#define PROC_RT_CRITICAL (1 << 4)

#define PROC_RT_PRIO_BOOST 10

inline static uint32_t __attribute__((always_inline)) get_priority(struct process *pctx)
{
    uint32_t ret = 0;
    ret += pctx->static_priority * pctx->dynamic_priority;
    ret += (pctx->real_time_priority >= PROC_RT_RECOMMEND) ? (PROC_RT_PRIO_BOOST * (pctx->real_time_priority >> 2)) : 0;
    ret += (pctx->real_time_priority >= PROC_RT_RECOMMEND && pctx->ticks_since_last_run >= pctx->desired_rt_interval) ? (PROC_RT_PRIO_BOOST * pctx->real_time_priority) : 0;
    return ret;
}

struct __attribute__((packed)) process {
    uint32_t pid;

    uint32_t locked;
    uint64_t lock_target;

    struct pagemap *proc_map;

    uint16_t static_priority;
    uint16_t dynamic_priority;

    uint32_t real_time_priority;
    uint32_t ticks_since_last_run;
    uint16_t desired_rt_interval;

#ifdef __x86_64__
    uint64_t rsp;
#endif

    uint32_t is_cur;

    struct process *next;
};

#endif
