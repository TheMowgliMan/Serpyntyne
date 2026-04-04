#include <stdint.h>
#include <terminal.h>

uint64_t seed;

static inline uint64_t rdtsc(void)
{
    uint32_t eax, edx;
    asm volatile("rdtsc\n\t" : "=a" (eax), "=d" (edx));
    return (uint64_t)eax | (uint64_t)edx << 32;
}

void rdtsc_seed_rand(void)
{
    seed = (~seed & UINT64_C(0xEFB38A9C0D39F73A)) ^ (rdtsc() + seed) * 54477213871ul;
    seed *= 0x2545F4914F6CDD1Dull;
}

uint64_t random(void)
{
    if (seed % 100 == 0)
    {
        seed = (~seed & UINT64_C(0xEFB38A9C0D39F73A)) ^ (rdtsc() + seed) * 54477213871ul;
    }
    else
    {
        seed *= 0x2545F4914F6CDD1Dull;
    }

    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;
    return seed;
}

uint64_t compress_to_range(uint64_t n, uint64_t start, uint64_t end)
{
    uint64_t r = n % (end - start + 1);

    r += start;

    return r;
}

uint64_t randrange(uint64_t start, uint64_t end)
{
    uint64_t r = random();
    r = compress_to_range(r, start, end);
    return r;
}


void init_rand(void)
{
    seed = rdtsc() + 993319 + (rdtsc() << 33);
    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;
    seed *= 0x2545F4914F6CDD1Dull;

    for (int i = 0; i < randrange(100, 5000); i++) { rdtsc_seed_rand(); }
}

void seed_rand(uint64_t nseed)
{
    seed = (~seed & UINT64_C(0xEFB38A9C0D39F73A)) ^ (nseed + seed) * 54477213871ul;
    seed *= 0x2545F4914F6CDD1Dull;
}
