#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include <stdint.h>
#include <stddef.h>

extern void isr0x00;
extern void isr0x01;
extern void isr0x02;
extern void isr0x03;
extern void isr0x04;
extern void isr0x05;
extern void isr0x06;
extern void isr0x07;
extern void isr0x08;
extern void isr0x0A;
extern void isr0x0B;
extern void isr0x0C;
extern void isr0x0D;
extern void isr0x0E;
extern void isr0x0F;
extern void isr0x10;
extern void isr0x11;
extern void isr0x12;
extern void isr0x13;
extern void isr0x14;
extern void isr0x15;
extern void isr0x1C;
extern void isr0x1D;
extern void isr0x1E;

struct intframe
{
  uint64_t r15;
  uint64_t r14;
  uint64_t r13;
  uint64_t r12;
  uint64_t r11;
  uint64_t r10;
  uint64_t r9;
  uint64_t r8;
  uint64_t rbp;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t rdx;
  uint64_t rcx;
  uint64_t rbx;
  uint64_t rax;
  uint64_t vector;
  uint64_t err;
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
};

void handle_interrupt(struct intframe* infr);

#endif
