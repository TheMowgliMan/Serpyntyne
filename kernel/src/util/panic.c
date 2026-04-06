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
