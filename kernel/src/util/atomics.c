/* This file uses `_Atomic` and so must be C11 or newer */

#include <util/atomics.h>
#include <archutil/asmstubs.h>
#include <util/panic.h>
#include <terminal.h>

void initSpinlock(spinlock_t *lock)
{
    __atomic_store_n(&lock->lock, 0, __ATOMIC_RELAXED);
    lock->open_attempts = 0;
    lock->owner_pid = 0;
}

void acquireSpinlock(spinlock_t *lock, uint32_t pid)
{
    if (pid = lock->owner_pid) return;

    while (true)
    {
        while (__atomic_load_n(&lock->lock, __ATOMIC_RELAXED))
            pause();

        if (!__atomic_exchange_n(&lock->lock, 1, __ATOMIC_ACQUIRE))
            break;

        increment_open_attempts(lock);

        if (count > 1000000000) exception(SPINLOCK_LONG_HOLD, (int64_t)count, (int64_t)get_owner_pid(lock));
    }

    set_owner_pid(lock, pid);
}

void releaseSpinlock(spinlock_t *lock)
{
    __atomic_store_n(&lock->lock, 0, __ATOMIC_RELEASE);
    lock->open_attempts = 0;
    lock->owner_pid = 0;
}

void testSpinlock(spinlock_t *lock)
{
    return __atomic_load_n(&lock->lock, __ATOMIC_RELAXED);
}
