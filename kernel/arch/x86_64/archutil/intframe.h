#ifndef INTFRAME_H_
#define INTFRAME_H_

#include <stdint.h>
#include <stdbool.h>
#include <terminal.h>

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

inline uint64_t __attribute__((always_inline)) get_vector(struct intframe* iframe)
{
    return iframe->vector;
}

inline bool __attribute__((always_inline)) is_fatal(struct intframe* iframe)
{
    uint8_t v = (uint8_t)iframe->vector;
    if (v == 0x2 || v == 0x8 || v == 0x0a || v == 0x0b || v == 0x0e) return true;
    return false;
}

inline bool __attribute__((always_inline)) is_exception(struct intframe* iframe)
{
    if (iframe->vector < 32) return true;
    return false;
}

inline uint64_t __attribute__((always_inline)) get_error_code(struct intframe* iframe)
{
    return iframe->err;
}

inline void __attribute__((always_inline)) print_registers(struct intframe* iframe)
{
    kprintf("RIP: %x RSP: %x\r\n", (int64_t)(iframe->rip), (int64_t)(iframe->rsp));
    kprintf("RFlags: %b\r\n", (int64_t)(iframe->rflags));
    kprintf("RAX: %x RBX: %x RCX: %x RDX: %x\r\n",
            (int64_t)(iframe->rax),
            (int64_t)(iframe->rbx),
            (int64_t)(iframe->rcx),
            (int64_t)(iframe->rdx));
    kprintf("RSI: %x RDI: %x RBP: %x R8: %x\r\n",
            (int64_t)(iframe->rsi),
            (int64_t)(iframe->rdi),
            (int64_t)(iframe->rbp),
            (int64_t)(iframe->r8));
    kprintf("R9: %x R10: %x R11: %x R12: %x\r\n",
            (int64_t)(iframe->r9),
            (int64_t)(iframe->r10),
            (int64_t)(iframe->r11),
            (int64_t)(iframe->r12));
    kprintf("R13: %x R14: %x R15: %x CS: %x\r\n",
            (int64_t)(iframe->r13),
            (int64_t)(iframe->r14),
            (int64_t)(iframe->r15),
            (int64_t)(iframe->cs));
}

#endif
