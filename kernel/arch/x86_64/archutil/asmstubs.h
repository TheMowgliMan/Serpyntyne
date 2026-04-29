#ifndef ASSEMBLY_STUBS_H
#define ASSEMBLY_STUBS_H

#include <stdint.h>

inline void __attribute__((always_inline)) pause(void)
{
    asm volatile ("pause");
}

inline void __attribute__((always_inline)) load_cr3(uintptr_t cr3_value)
{
    asm volatile ("movq %0, %%cr3" : : "r" ((uint64_t)cr3_value));
    //asm volatile ("int $0x0e");
}

uint64_t read_cr2(void);

#endif
