#ifndef RANDOM_H_
#define RANDOM_H_

#include <stdint.h>
#include <atomics.h>

struct randomInstance {
    uint64_t seed;
    spinlock_t lock;
}

void init_rand(void);
void rdtsc_seed_rand(void);
void seed_rand(uint64_t nseed);

void random(void);
uint64_t randrange(uint64_t start, uint64_t end);
int64_t compress_to_range(int64_t n, int64_t start, int64_t end);

#endif
