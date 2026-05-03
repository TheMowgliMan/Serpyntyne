#include <util/postinit.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <terminal.h>
#include <util/random.h>

#include <uacpi/uacpi.h>
#include <uacpi/tables.h>
#include <uacpi/log.h>
#include <uacpi/context.h>

void postInit(void)
{
    klog(LOG_PROC, "Post-initializing...\r\n");

    init_rand();

    uint64_t free_ram = initPmm();
    klog(LOG_NOTICE, "Usable RAM: %d bytes\r\n", (int64_t)free_ram);

    paging_init();

    heap_init();

    klog(LOG_PROC, "Starting uACPI...\r\n");

    void *uacpi_early_buf = krmalloc(32768);

    uacpi_context_set_log_level(UACPI_LOG_DEBUG);

    uacpi_setup_early_table_access(uacpi_early_buf, 32768);

    if (!uacpi_table_subsystem_available())
        klog(LOG_ERROR, "uACPI table subsytem is unavailable!");

    uacpi_table *madt = krmalloc(4096);
    kprintf("jumped out\r\n");
    uacpi_table_find_by_signature("APIC", madt);

    klog(LOG_SUCCESS, "Post-init complete!\r\n");
}
