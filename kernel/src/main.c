#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <terminal.h>
#include <archinit.h>
#include <util/random.h>

// Set the base revision to 6, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        ;
    }
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)
  {
	  hcf();
  }

  seed_rand();

  termInit();
  kputs("Serpyntyne 0.0.0\r\n");
  
  arch_preinit();

  // We're done, just hang...
  kerror("We're done, hanging...\r\n");

  kprintf("%d %d %d \r\n", randrange(1, 10), randrange(1, 10), randrange(1, 10));

  asm volatile ("int $0x09");

  hcf();
}
