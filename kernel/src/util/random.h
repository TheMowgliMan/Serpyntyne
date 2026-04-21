#ifndef RANDOM_H_
#define RANDOM_H_

#include <stdint.h>
#include <util/atomics.h>

struct randomInstance {
    uint64_t seed;
    spinlock_t lock;
};

void init_rand(void);
void init_rand_instance(struct randomInstance *ri);
void rdtsc_seed_rand(void);
void rdtsc_seed_rand_instance(struct randomInstance *ri);

void seed_rand(uint64_t nseed);
void seed_rand_instance(struct randomInstance *ri, uint64_t nseed);

uint64_t random(void);
uint64_t random_instance(struct randomInstance *ri);
uint64_t randrange(uint64_t start, uint64_t end);
uint64_t randrange_instance(struct randomInstance *ri, uint64_t start, uint64_t end);

uint64_t compress_to_range(uint64_t n, uint64_t start, uint64_t end);

#endif
