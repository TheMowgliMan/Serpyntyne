#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <util/liminereq.h>
#include <limine.h>

#include <terminal.h>
#include <archinit.h>
#include <util/forthefunni.h>
#include <util/postinit.h>

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm volatile ("hlt");
    }
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
  initializeLimineRequests();
  kputs("Version 0.0.0\r\n");
  
  arch_preinit();
  postInit();

  kprintf("Serpyntyne: %s \r\n", getStartMessage());

  // We're done, just hang...
  kerror("We're done, hanging...\r\n");

  hcf();
}
