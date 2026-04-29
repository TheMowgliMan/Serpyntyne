#include <archutil/asmstubs.h>
#include <stdint.h>

uint64_t read_cr2(void)
{
    uint64_t ret;
    asm volatile ("mov %%cr2, %0" : "=r" (ret) : : "memory");
    return ret;
}
