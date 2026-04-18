#ifndef ASSEMBLY_STUBS_H_
#define ASSEMBLY_STUBS_H_

inline void __attribute__((always_inline)) pause(void)
{
    asm volatile ("pause");
}

void load_cr3( void* cr3_value )
{
    asm volatile("mov %0, %%cr3" :: "r"((uint64_t)cr3_value) : "memory");
}

#endif
