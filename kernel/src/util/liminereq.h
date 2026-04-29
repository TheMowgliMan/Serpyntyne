#ifndef LIMINE_REQ_H
#define LIMINE_REQ_H

#include <limine.h>
#include <terminal.h>
#include <util/panic.h>

extern struct limine_memmap_response *memmap_response;
extern struct limine_hhdm_response *hhdm_response;
extern struct limine_framebuffer_response *framebuffer_response;
extern struct limine_executable_file_response *executable_file_response;
extern struct limine_executable_address_response *executable_address_response;

extern struct limine_framebuffer *limine_framebuffer_ctx;

void initializeLimineRequests(void);

#endif
