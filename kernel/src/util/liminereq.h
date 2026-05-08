#ifndef LIMINE_REQ_H
#define LIMINE_REQ_H

#include <limine.h>
#include <terminal.h>
#include <util/panic.h>

#define PHYS_TO_HHDM(X) ((void*)((X) + hhdm_response->offset))
#define HHDM_TO_PHYS(X) ((uintptr_t)(X) - hhdm_response->offset)

#define CPU_COUNT (mp_response->cpu_count)
#ifdef __x86_64__
#define BSP_CPU_LAPIC_ID (mp_response->bsp_lapic_id)
#endif // !x86-64
#define MP_FLAGS (mp_response->flags)
#define GET_CPU(X) (mp_response->cpus[X])
#ifdef __x86_64__
#define GET_LAPIC_ID(X) (X->lapic_id)
#endif

#define TSC_FREQUENCY (tsc_frequency_response->frequency)
#define TIME_BETWEEN(X, Y) (((Y) - (X)) / (TSC_FREQUENCY / 1000000)) // In microseconds

extern struct limine_memmap_response *memmap_response;
extern struct limine_hhdm_response *hhdm_response;
extern struct limine_framebuffer_response *framebuffer_response;
extern struct limine_executable_file_response *executable_file_response;
extern struct limine_executable_address_response *executable_address_response;
extern struct limine_rsdp_response *rsdp_response;
extern struct limine_mp_response *mp_response;
extern struct limine_tsc_frequency_response *tsc_frequency_response;

extern struct limine_framebuffer *limine_framebuffer_ctx;

void initializeLimineRequests(void);

#endif
