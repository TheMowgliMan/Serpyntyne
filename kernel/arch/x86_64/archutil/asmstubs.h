#ifndef ASSEMBLY_STUBS_H
#define ASSEMBLY_STUBS_H

inline void __attribute__((always_inline)) pause(void)
{
    asm volatile ("pause");
}

void load_cr3( void* cr3_value );

#endif
