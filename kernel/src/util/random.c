#include <stdint.h>

uint64_t seed;

static inline uint64_t rdtsc(void)
{
    uint32_t eax, edx;
    asm volatile("rdtsc\n\t", "=a" (eax), "=d" (edx));
    return (uint64_t)eax | (uint64_t)edx << 32;
}

void seed_rand(void)
{
    seed = rdtsc() * ~(rdtsc << 17);
}

uint64_t random(void)
{
    seed = seed * (~(rdtsc << 17) ^ seed) + seed;
    return seed;
}

int64_t randrange(int64_t start, int64_t end)
{
    unt64_t r = (int64_t)random();
    r = r % (end - start + 1);
    r = r - start;
    return r;
}
