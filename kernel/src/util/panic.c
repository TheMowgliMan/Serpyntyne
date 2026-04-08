#include <util/panic.h>
#include <stdint.h>
#include <terminal.h>
#include <util/forthefunni.h>
#include <archutil/abort.h>
#include <archutil/intframe.h>

void panic(struct intframe* iframe)
{
    // This file doesn't actually know what 'iframe' looks like, as it's architecture-specific.
    // Therefore, any interaction with it must be mediated by archutil functions.
    kprintf("\033[41mPanic: %s\r\n\033[41m", getPanicMessage());
    kprintf("Error code: %b ", (int64_t)get_error_code(iframe));
    kprintf("Vector: %x\r\n", (int64_t)get_vector(iframe));

    print_registers(iframe);

    kernel_abort();
}

void exception(uint64_t reason, int64_t data1, int64_t data2)
{
    kprintf("\033[41mKernel Exception: %s\r\n\033[41m", getPanicMessage());
    kprintf("Reason: %x ", (int64_t)reason);

    if (reason == SPINLOCK_LONG_HOLD)
    {
        kprintf("(SPINLOCK_LONG_HOLD)\r\n");
        kprintf("A spinlock was held for an extremely long time. It was held by process #%d with %d attempts of acquiring it.\r\n", (int64_t)data2, (int64_t)data1);

        if (data2 == 0)
        {
            kprintf("The spinlock was held by the kernel; as such, a system lockup is likely.\r\nThe system is aborting, restart it manually.\r\n");
            kernel_abort();
        }
    }
}
