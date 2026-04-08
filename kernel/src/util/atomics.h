#include <stdint.h>

/* First 32 bits: PID of locker, 0 if kernel. Next 31 bits: lock attempts. Last bit: lock itself. */
/* These masks are set-oriented: they blank a region about to be set. NOT them to get data on what only they mask. */
#define SPINLOCK_LOCK_MASK 0xFFFFFFFFFFFFFFFEllu
#define SPINLOCK_OWNER_PID_MASK 0x00000000FFFFFFFFllu
#define SPINLOCK_OPEN_ATTEMPTS_MASK 0xFFFFFFFF00000001llu

typedef _Atomic(unsigned long int) atomic_ulint_t;
typedef _Atomic(unsigned int) atomic_uint_t;

typedef struct {
    atomic_ulint_t lock;
    atomic_uint_t open_attempts;
} spinlock_t;

inline uint32_t __attribute__((always_inline)) get_owner_pid(spinlock_t* lock)
{
    return (uint32_t)(((uint64_t)(lock->lock) & ~SPINLOCK_OWNER_PID_MASK) >> 32);
}

inline void __attribute__((always_inline)) set_owner_pid(spinlock_t *lock, uint32_t pid)
{
    lock->lock = (((uint64_t)(lock->lock) & SPINLOCK_OWNER_PID_MASK) | ((uint64_t)pid << 32));
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
