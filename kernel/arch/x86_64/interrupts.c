#include <interrupts.h>
#include <terminal.h>
#include <stdint.h>

void handle_interrupt(struct intframe* infr)
{
    kputs("Interrupt recieved:\r\n");
    kprintf("Vector: %x \r\n", (int64_t)infr->vector);
    asm volatile ("sti");
}
