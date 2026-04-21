#ifndef ATOMICS_H_
#define ATOMICS_H_

#include <stdint.h>

#define LOCK_SIZE 8

typedef _Atomic(unsigned long int) atomic_ulint_t;
typedef _Atomic(unsigned int) atomic_uint_t;

typedef struct {
    uint64_t lock;
} spinlock_t;

void initSpinlock(spinlock_t *lock);
void acquireSpinlock(spinlock_t *lock, uint32_t pid);
void releaseSpinlock(spinlock_t *lock);
uint64_t testSpinlock(spinlock_t *lock);

#endif
