#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include <stdint.h>
#include <stddef.h>
#include <archutil/intframe.h>

extern void isr0x00(void);
extern void isr0x01(void);
extern void isr0x02(void);
extern void isr0x03(void);
extern void isr0x04(void);
extern void isr0x05(void);
extern void isr0x06(void);
extern void isr0x07(void);
extern void isr0x08(void);
extern void isr0x09(void);
extern void isr0x0A(void);
extern void isr0x0B(void);
extern void isr0x0C(void);
extern void isr0x0D(void);
extern void isr0x0E(void);
extern void isr0x0F(void);
extern void isr0x10(void);
extern void isr0x11(void);
extern void isr0x12(void);
extern void isr0x13(void);
extern void isr0x14(void);
extern void isr0x15(void);
extern void isr0x1C(void);
extern void isr0x1D(void);
extern void isr0x1E(void);

void handle_interrupt(struct intframe* infr);

#endif
