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
    if (v == 0x2 || v == 0x8 || v == 0x0a || v == 0x0b) return true;
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

inline bool __attribute__((always_inline)) is_placeholder(struct intframe* iframe)
{
    uint8_t v = (uint8_t)iframe->vector;

    if (v == 0x09) return true;
    return false;
}

inline bool __attribute__((always_inline)) is_debug(struct intframe *iframe)
{
    uint8_t v = (uint8_t)iframe->vector;

    if (v == 0x01) return true;
    return false;
}

void print_registers(struct intframe* iframe);
char *get_exception_name(uint8_t vector);

inline void __attribute__((always_inline)) raise_debug_interrupt(void)
{
    asm volatile ("int $0x01");
}

#endif
