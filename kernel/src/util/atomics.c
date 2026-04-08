/* This file uses `_Atomic` and so must be C11 or newer */

#include <util/atomics.h>
#include <archutil/asmstubs.h>
#include <util/panic.h>
#include <terminal.h>

void initSpinlock(spinlock_t *lock)
{
    lock->lock = 0x0000000000000000;
    lock->open_attempts = 0;
}

void acquireSpinlock(spinlock_t *lock, uint32_t pid)
{
    if ((lock->lock & ~SPINLOCK_LOCK_MASK) == 1) // Locked!
    {
        while (true)
        {
            pause();

            if ((lock->lock & ~SPINLOCK_LOCK_MASK) == 0)
            {
                break;
            }

            increment_open_attempts(lock);

            if (count > 1000000000) exception(SPINLOCK_LONG_HOLD, (int64_t)count, (int64_t)get_owner_pid(lock));
        }
    }

    lock->lock = (atomic_ulint_t)(~SPINLOCK_LOCK_MASK);
    set_owner_pid(lock, pid);
}

void releaseSpinlock(spinlock_t *lock)
{
    initSpinlock(lock);
}
