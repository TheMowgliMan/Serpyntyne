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
    klog(LOG_ERROR, "Kernel panic!\r\n");
    switch_to_panic_bg();

    kprintf(TTY_REDBG "Panic: %s" TTY_REDBG, getPanicMessage()); pad_with_spaces();
    kprintf("\r\nError code: %b ", (int64_t)get_error_code(iframe));
    uint64_t v = get_vector(iframe);
    kprintf("Vector: %x", (int64_t)v); pad_with_spaces();
    kprintf("\r\n(%s)", get_exception_name(v)); pad_with_spaces();

    if (is_placeholder(iframe))
    {
        kprintf("\r\nNote that this is a placeholder interrupt routine."); pad_with_spaces(); kprintf("\r\nPlease find the culprit and add a proper panic handler.");
    }

    print_registers(iframe);

    kernel_abort();
}

void exception(uint64_t reason, int64_t data1, int64_t data2)
{
    klog(LOG_ERROR, "Kernel error!\r\n");
    kprintf(TTY_REDBG "Exception: %s\r\n" TTY_RED, getPanicMessage());
    kprintf("Reason: %x ", (int64_t)reason);

    if (reason == NO_MEMORY)
    {
        kprintf("(NO_MEMORY)\r\n");
        kprintf("There was no memory available to the system to use for the allocator.\r\nThe system is aborting, please restart it manually.\r\n");
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
        kprintf(TTY_RESETBG TTY_MAGENTA "This architecture supports pages of size %d or %d." TTY_RED "The system is aborting, please restart it manually.\r\n",
                0ll, (int64_t)(LARGE_PAGE_SIZE_EXPONENT));
        kernel_abort();
    }

    if (reason == ILLEGAL_PAGE_MAP_SIZE)
    {
        kprintf("(ILLEGAL_PAGE_MAP_SIZE)\r\n");
        kprintf("A map to a page of an impossible size (%d) was requested for virtual address %x.\r\nThe system is aborting, please restart it manually.\r\n",
                data1, data2);
        kernel_abort();
    }

    if (reason == OUT_OF_MEMORY)
    {
        kprintf("(OUT_OF_MEMORY)\r\n");
        kprintf("The system ran out of memory during a critical operation. See below for more details.\r\nThe system is aborting, please restart it manually\r\n");
        kprintf("Cause: ");

        switch (data1)
        {
            case OOM_PMM_FRAME_CREATION:
                kprintf("(OOM_PMM_FRAME_CREATION)\r\nNo place was found for physical memory manager frame data.\r\n");
                break;
            case OOM_PMM_SPINLOCKS_CREATION:
                kprintf("(OOM_PMM_SPINLOCKS_CREATION)\r\nNo place was found for physical memory manager spinlocks.\r\n");
                break;
            default:
                kprintf("(other cause)\r\nNo memory was found for a miscellanious system operation.\r\nWhen the cause is determined, please submit for it to be added to the panic list.\r\n");
        }

        kernel_abort();
    }

    if (reason == MISALIGNED_PAGE)
    {
        kprintf("(MISALIGNED_PAGE)\r\n");
        kprintf("A misaligned page was attempted to be mapped, virtual address %x, physical address %x!\r\nThe system is aborting, please restart it manually.\r\n",
                data1, data2);
        kernel_abort();
    }
}
