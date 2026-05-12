#include <util/liminereq.h>

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

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_stack_size_request stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST_ID,
    .revision = 0,
    .stack_size = 0x100000
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_executable_file_request executable_file_request = {
    .id = LIMINE_EXECUTABLE_FILE_REQUEST_ID,
    .revision = 0,
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_executable_address_request executable_address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_mp_request mp_request = {
    .id = LIMINE_MP_REQUEST_ID,
    .revision = 0,
    .flags = 0 // x2APIC isn't implemented yet
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_tsc_frequency_request tsc_frequency_request = {
    .id = LIMINE_TSC_FREQUENCY_REQUEST_ID,
    .revision = 0,
    .response = NULL // So we can abort if it doesn't exist (we don't use PIT calibration for now)
};

struct limine_memmap_response *memmap_response;
struct limine_hhdm_response *hhdm_response;
struct limine_framebuffer_response *framebuffer_response;
struct limine_executable_file_response *executable_file_response;
struct limine_executable_address_response *executable_address_response;
struct limine_rsdp_response *rsdp_response;
struct limine_mp_response *mp_response;
struct limine_tsc_frequency_response *tsc_frequency_response;

struct limine_framebuffer *limine_framebuffer_ctx = NULL;

uintptr_t HHDM_OFFSET;

void initializeLimineRequests(void)
{
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)
    {
        for (;;) { ; }
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
        || framebuffer_request.response->framebuffer_count < 1)
    {
        for (;;) { ; }
    }

    framebuffer_response = framebuffer_request.response;

    // Fetch the first framebuffer.
    limine_framebuffer_ctx = framebuffer_request.response->framebuffers[0];

    if (memmap_request.response == NULL) exception(NO_MEMORY, 0, 0);

    if (hhdm_request.response == NULL) exception(NO_HHDM, 0, 0);

    memmap_response = memmap_request.response;
    hhdm_response = hhdm_request.response;
    HHDM_OFFSET = hhdm_response->offset;

    executable_file_response = executable_file_request.response;
    executable_address_response = executable_address_request.response;

    rsdp_response = rsdp_request.response;

    mp_response = mp_request.response;
    tsc_frequency_response = tsc_frequency_request.response;
}
