#include <interrupts.h>

#include <terminal.h>
#include <memory.h>

#include <util/random.h>
#include <util/panic.h>

#include <archutil/intframe.h>

#include <acpi/acpi.h>

#include <process/scheduler.h>

#include <stdint.h>

extern scheduler_t *bsp_sched_ctx;

void handle_interrupt(struct intframe* infr)
{
    rdtsc_seed_rand(); // Adds just a little more entropy

    if (is_exception(infr))
    {
        jailbreak_terminal();
        panic(infr);
    }

    if (is_apic_timer(infr))
    {
        //jailbreak_terminal();
        //kprintf("%x\r\n", (int64_t)bsp_sched_ctx);
        asm volatile ("cli");
        scheduler_tick(bsp_sched_ctx);
        APIC_EOI();
    }
}
