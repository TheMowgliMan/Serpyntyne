#include <gdt.h>
#include <idt.h>
#include <terminal.h>

void arch_preinit(void) {
    kputs("Pre-Initializing architecture details (AMD64)...\r\n");

    initGDT();
    initIDT();

    kputs("\033[0;32mPre-init complete!\033[0;37m\r\n");
}
