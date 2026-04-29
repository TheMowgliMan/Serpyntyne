#include <util/liminereq.h>
#include <limine.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <terminal.h>
#include <archinit.h>
#include <util/forthefunni.h>
#include <util/postinit.h>

/* TESTING INCLUDES */

#include <mm/vmm.h>
#include <paging.h>
#include <memory.h>

/* END TESTING INCLUDES */

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
  termInit();
  kputs("Version 0.0.0\r\n");
  
  arch_preinit();
  postInit();

  kprintf("Serpyntyne: %s \r\n", getStartMessage());

  klog(LOG_PROC, "Testing `allocate_random_and_map`, with test string \"this was memcpy'd to an allocated area!\"\r\n");

  char str[64] = "this was memcpy'd to an allocated area!";

  char *space = (char*)allocate_random_and_map(kernel_page_table, NULL, 64, 0xFFFFFFFF00000000ull, MAP_NEWMAP | MAP_READABLE | MAP_WRITABLE | MAP_KERNEL);
  memset(space, 0, PAGE_SIZE);
  memcpy(space, str, 64);
  kprintf("%s\r\n", space);

  // We're done, just hang...
  klog(LOG_ERROR, "We're done, hanging...\r\n");

  hcf();
}
