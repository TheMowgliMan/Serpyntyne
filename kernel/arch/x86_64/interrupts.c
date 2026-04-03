#include <interrupts.h>
#include <terminal.h>

void handle_interrupt(struct intframe* infr)
{
    kputs("interrupt recieved...\r\n");
}
