#include <uacpi/kernel_api.h>
#include <uacpi/status.h>
#include <uacpi/log.h>
#include <uacpi/types.h>

#include <terminal.h>

#include <mm/vmm.h>
#include <paging.h>
#include <mm/heap.h>

#include <util/align.h>
#include <util/liminereq.h>

#include <stdint.h>

/*
 * This file is the central hub for all implementations of what's in kernel_api.h for uacpi.
 */

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address)
{
    out_rsdp_address = (uacpi_phys_addr*)((uint8_t *)(rsdp_response->address) - hhdm_response->offset);
    return UACPI_STATUS_OK;
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len)
{
    uintptr_t floor = (uintptr_t)addr;
    uintptr_t cieling = (uintptr_t)addr + len;

    uint64_t difference = floor - ALIGN_DOWN(floor, PAGE_SIZE);
    floor = ALIGN_DOWN(floor, PAGE_SIZE);
    cieling = ALIGN_UP(cieling, PAGE_SIZE);

    uint8_t *tmp = (uint8_t *)knock_a_few_bytes_off_the_old_heap_block(floor, cieling - floor);
    return (void*)(tmp + difference);
}

void uacpi_kernel_unmap(void *addr, uacpi_size len)
{
    uintptr_t floor = (uintptr_t)addr;
    uintptr_t cieling = (uintptr_t)addr + len;

    floor = ALIGN_DOWN(floor, PAGE_SIZE);
    cieling = ALIGN_UP(cieling, PAGE_SIZE);

    free_sized((void*)floor, cieling - floor);
}

void uacpi_kernel_log(uacpi_log_level log_level, const uacpi_char *c)
{
    switch (log_level) {
        case UACPI_LOG_ERROR:
            klog(LOG_ERROR, "uACPI: ");
            break;
        case UACPI_LOG_WARN:
            klog(LOG_NOTICE, "uACPI: ");
            break;
        default:
            klog(LOG_INFO, "uACPI: ");
    }

    kprintf(c);
    kputs("\r\n");
}
