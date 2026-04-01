#include <gdt.h>
#include <terminal.h>

void arch_preinit(void) {
    kputs("Pre-Initializing architecture details (AMD64)...\r\n");

    initGDT();
}
