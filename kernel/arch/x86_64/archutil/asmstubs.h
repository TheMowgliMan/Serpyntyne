#ifndef ASSEMBLY_STUBS_H_
#define ASSEMBLY_STUBS_H_

inline void __attribute__((always_inline)) pause(void)
{
    asm volatile ("pause");
}

#endif
