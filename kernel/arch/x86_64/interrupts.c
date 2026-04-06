#include <interrupts.h>
#include <terminal.h>
#include <stdint.h>
#include <memory.h>
#include <util/random.h>
#include <archutil/intframe.h>
#include <util/panic.h>

void handle_interrupt(struct intframe* infr)
{
    rdtsc_seed_rand(); // Adds just a little more entropy

    if (is_exception(infr))
    {
        panic(infr);
    }

    asm volatile ("sti");
}
