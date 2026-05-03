#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include <stdint.h>
#include <stddef.h>
#include <archutil/intframe.h>

#define GET_ISR(x) ((void (*)())isr_array[x])

extern uint64_t isr_array[256];

void handle_interrupt(struct intframe* infr);

#endif
