#ifndef ATOMICS_H_
#define ATOMICS_H_

#include <stdint.h>

typedef _Atomic(unsigned long int) atomic_ulint_t;
typedef _Atomic(unsigned int) atomic_uint_t;

typedef struct {
    uint64_t lock;
    atomic_uint_t open_attempts;
    atomic_uint_t owner_pid;
} spinlock_t;

inline uint32_t __attribute__((always_inline)) get_owner_pid(spinlock_t* lock)
{
    return (uint32_t)(lock->owner_pid);
}

inline void __attribute__((always_inline)) set_owner_pid(spinlock_t *lock, uint32_t pid)
{
    lock->owner_pid = pid;
}

inline uint32_t __attribute__((always_inline)) get_open_attempts(spinlock_t *lock)
{
    return (uint32_t)(lock->open_attempts);
}

inline void __attribute__((always_inline)) set_open_attempts(spinlock_t *lock, uint32_t attempts)
{
    lock->open_attempts = (atomic_uint_t)attempts;
}

inline void __attribute__((always_inline)) increment_open_attempts(spinlock_t *lock)
{
    lock->open_attempts += 1;
}

void initSpinlock(spinlock_t *lock);
void acquireSpinlock(spinlock_t *lock, uint32_t pid);
void releaseSpinlock(spinlock_t *lock);
uint64_t testSpinlock(spinlock_t *lock);

#endif
