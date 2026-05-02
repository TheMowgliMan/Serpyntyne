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

static void uacpi_klog(int logstatus, const char* restrict format, ...)
{
#ifdef DEBUG
    klog(logstatus, TTY_HIMAGENTA "uACPI Kernel API: " TTY_RESET);

    va_list parameters;
    va_start(parameters, format);

    kvprintf(format, parameters);

    va_end(parameters);
#endif
}

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address)
{
    uacpi_klog(LOG_INFO, "Getting RSDP...\r\n");

    *out_rsdp_address = (uacpi_phys_addr)((uintptr_t)(rsdp_response->address) - hhdm_response->offset);

    uacpi_klog(LOG_INFO, "Got RSDP at %x!\r\n", (int64_t)*out_rsdp_address);

    return UACPI_STATUS_OK;
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len)
{
    /* This does not work, yet: */
    // uintptr_t floor = (uintptr_t)addr;
    // uintptr_t cieling = (uintptr_t)addr + len;
    //
    // uint64_t difference = floor - ALIGN_DOWN(floor, PAGE_SIZE);
    // floor = ALIGN_DOWN(floor, PAGE_SIZE);
    // cieling = ALIGN_UP(cieling, PAGE_SIZE);
    //
    // uacpi_klog(LOG_INFO, "Mapping addr %x, offset %d, to floor %x and cieling %x, with difference %x\r\n",
    //            (int64_t)addr, (int64_t)len, (int64_t)floor, (int64_t)cieling, (int64_t)difference);
    //
    // uintptr_t tmp = (uintptr_t)knock_a_few_bytes_off_the_old_heap_block(floor, cieling);
    // return (void*)(tmp + difference);

    /* This is confirmed to work: */
    return (void*)(addr + hhdm_response->offset);
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
            klog(LOG_ERROR, TTY_HIBLUE "uACPI: " TTY_RESET);
            break;
        case UACPI_LOG_WARN:
            klog(LOG_NOTICE, TTY_HIBLUE "uACPI: " TTY_RESET);
            break;
        default:
            klog(LOG_INFO, TTY_HIBLUE "uACPI: " TTY_RESET);
    }

    kprintf(c);
    kputs("\r");
}
