#include <gdt.h>
#include <idt.h>
#include <terminal.h>

void arch_preinit(void) {
    kputs(TTY_CYAN "Pre-Initializing architecture details (AMD64)...\r\n" TTY_RESET);

    initGDT();
    initIDT();

    kputs(TTY_GREEN "\033[0;32mPre-init complete!\033[0;37m\r\n" TTY_RESET);
}
