#include <gdt.h>
#include <idt.h>
#include <terminal.h>

void arch_preinit(void) {
    klog(LOG_PROC, TTY_CYAN "Pre-Initializing architecture details (AMD64)...\r\n" TTY_RESET);

    initGDT();
    initIDT();

    klog(LOG_SUCCESS, TTY_GREEN "Pre-init complete!\r\n" TTY_RESET);
}
