#include <stdint.h>
#include <terminal.h>

#include <util/atomics.h>

#include <util/random.h>

#include <archutil/asmstubs.h>

uint64_t seed;
spinlock_t global_random_instance_lock;
spinlock_t *gril;

void rdtsc_seed_rand_instance(struct randomInstance *ri)
{
    acquireSpinlock(&(ri->lock), 0);

    ri->seed = (~ri->seed & UINT64_C(0xEFB38A9C0D39F73A)) ^ (read_arch_time_stamp_counter() + ri->seed) * 54477213871ul;
    ri->seed = ri->seed * 0x2545F4914F6CDD1Dull;

    releaseSpinlock(&(ri->lock));
}

void rdtsc_seed_rand(void)
{
    acquireSpinlock(gril, 0);

    seed = (~seed & UINT64_C(0xEFB38A9C0D39F73A)) ^ (read_arch_time_stamp_counter() + seed) * 54477213871ul;
    seed *= 0x2545F4914F6CDD1Dull;

    releaseSpinlock(gril);
}

uint64_t random_instance(struct randomInstance *ri)
{
    acquireSpinlock(&(ri->lock), 0);

    if (ri->seed % 100 == 0)
    {
        ri->seed = (~ri->seed & UINT64_C(0xEFB38A9C0D39F73A)) ^ (read_arch_time_stamp_counter() + ri->seed) * 54477213871ul;
    }
    else
    {
        ri->seed *= 0x2545F4914F6CDD1Dull;
    }

    ri->seed ^= ri->seed >> 12;
    ri->seed ^= ri->seed << 25;
    ri->seed ^= ri->seed >> 27;

    uint64_t ret = ri->seed;

    releaseSpinlock(&(ri->lock));
    return ret;
}

uint64_t random(void)
{
    acquireSpinlock(gril, 0);

    if (seed % 100 == 0)
    {
        seed = (~seed & UINT64_C(0xEFB38A9C0D39F73A)) ^ (read_arch_time_stamp_counter() + seed) * 54477213871ul;
    }
    else
    {
        seed *= 0x2545F4914F6CDD1Dull;
    }

    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;

    uint64_t ret = seed;
    releaseSpinlock(gril);

    return ret;
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

uint64_t randrange_instance(struct randomInstance *ri, uint64_t start, uint64_t end)
{
    // This funtion doesn't feature the lock as it's included in random_instance()
    uint64_t r = random_instance(ri);

    r = compress_to_range(r, start, end);
    return r;
}

void init_rand(void)
{
    gril = &global_random_instance_lock;
    initSpinlock(gril);

    acquireSpinlock(gril, 0);

    seed = read_arch_time_stamp_counter() + 993319 + (read_arch_time_stamp_counter() << 33);
    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;
    seed *= 0x2545F4914F6CDD1Dull;

    releaseSpinlock(gril);

    for (unsigned int i = 0; i < randrange(100, 5000); i++) { rdtsc_seed_rand(); }
}

void init_rand_instance(struct randomInstance *ri)
{
    initSpinlock(&(ri->lock));

    acquireSpinlock(&(ri->lock), 0);

    ri->seed = read_arch_time_stamp_counter() + 993319 + (read_arch_time_stamp_counter() << 33);
    ri->seed ^= ri->seed >> 12;
    ri->seed ^= ri->seed << 25;
    ri->seed ^= ri->seed >> 27;
    ri->seed *= 0x2545F4914F6CDD1Dull;

    releaseSpinlock(&(ri->lock));

    /*
     * We intentionally use the global randrange() rather than the local randrange_instance()
     * as it has probably collected more entropy at this point. In addition, rdtsc() has
     * advanced suitably far that it will add a lot more entropy than it did when we were
     * initializing the global random instance.
     */
    for (unsigned int i = 0; i < randrange(100, 5000); i++) { rdtsc_seed_rand_instance(ri); }
}

void seed_rand(uint64_t nseed)
{
    acquireSpinlock(gril, 0);

    seed = (~seed & UINT64_C(0xEFB38A9C0D39F73A)) ^ (nseed + seed) * 54477213871ul;
    seed *= 0x2545F4914F6CDD1Dull;

    releaseSpinlock(gril);
}

void seed_rand_instance(struct randomInstance *ri, uint64_t nseed)
{
    acquireSpinlock(&(ri->lock), 0);

    ri->seed = (~ri->seed & UINT64_C(0xEFB38A9C0D39F73A)) ^ (nseed + ri->seed) * 54477213871ul;
    ri->seed *= 0x2545F4914F6CDD1Dull;

    releaseSpinlock(&(ri->lock));
}
