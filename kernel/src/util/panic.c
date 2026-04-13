#include <util/panic.h>
#include <stdint.h>
#include <terminal.h>
#include <util/forthefunni.h>
#include <archutil/abort.h>
#include <archutil/intframe.h>
#include <archutil/defines.h>

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
        kprintf("A spinlock was held for an extremely long time. It was held by process #%d with %d attempts of acquiring it.\r\n", data2, data1);

        if (data2 == 0)
        {
            kprintf("The spinlock was held by the kernel; as such, a system lockup is likely.\r\nThe system is aborting, restart it manually.\r\n");
            kernel_abort();
        }
    }

    if (reason == NO_MEMORY)
    {
        kprintf("(NO_MEMORY)\r\n");
        kprintf("There was no memory available to the system to use.\r\nThe system is aborting, please restart it manually.\r\n");
        kernel_abort();
    }

    if (reason == NO_HHDM)
    {
        kprintf("(NO_HHDM)\r\n");
        kprintf("There is no Higher Half Direct Map.\r\nThe system is aborting, please restart it manually.\r\n");
        kernel_abort();
    }

    if (reason == BAD_FRAME_SIZE)
    {
        kprintf("(BAD_FRAME_SIZE)\r\n");
        kprintf("A bad frame at physical address %x was reported with size %d (%d bytes), which is not supported by this architecture (" ARCH_NAME ").\r\n",
                data2, data1, (int64_t)((1ull << data1) * PAGE_SIZE));
        kprintf("This architecture supports pages of size %d or %d. The system is aborting, please restart it manually\r\n",
                0ll, (int64_t)(LARGE_PAGE_SIZE_EXPONENT));
        kernel_abort();
    }
}
