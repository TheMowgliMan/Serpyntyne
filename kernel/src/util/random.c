#include <stdint.h>

uint64_t seed;

static inline uint64_t rdtsc(void)
{
    uint32_t eax, edx;
    asm volatile("rdtsc\n\t" : "=a" (eax), "=d" (edx));
    return (uint64_t)eax | (uint64_t)edx << 32;
}

void seed_rand(void)
{
    seed = rdtsc() + 993319 + (rdtsc() << 33);
    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;
    seed *= 0x2545F4914F6CDD1Dull;
}

uint64_t random(void)
{
    seed = (~seed & UINT64_C(0xEFB38A9C0D39F73A)) ^ (rdtsc() + seed) * 54477213871ul;

    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;
    return seed;
}

int64_t randrange(int64_t start, int64_t end)
{
    uint64_t r = (int64_t)random();
    r = r % (end - start + 1);
    r = r - start;
    return r;
}
