#include <archutil/asmstubs.h>
#include <stdint.h>

void load_cr3( void* cr3_value )
{
    asm volatile("mov %0, %%cr3" :: "r"((uint64_t)cr3_value) : "memory");
}
