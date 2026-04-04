#include <interrupts.h>
#include <terminal.h>
#include <stdint.h>
#include <memory.h>
#include <util/random.h>

void handle_interrupt(struct intframe* infr)
{
    rdtsc_seed_rand(); // Adds just a little more entropy

    char c[64];
    uint8_t type = 0;
    if (infr->vector < 32)
    {
        char msg[] = "Exception";
        memcpy(&c, &msg, sizeof(char) * strlen(msg));
        type = 1;
    }
    else
    {
        char msg[] = "Interrupt";
        memcpy(&c, &msg, sizeof(char) * strlen(msg));
    }

    kprintf("%s recieved:\r\n", c);
    kprintf("Vector: %x \r\n", (int64_t)infr->vector);
    asm volatile ("sti");
}
