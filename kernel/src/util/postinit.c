#include <util/postinit.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <terminal.h>
#include <util/random.h>

#include <uacpi/uacpi.h>

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
    uacpi_setup_early_table_access(uacpi_early_buf, 32768);

    klog(LOG_SUCCESS, "Post-init complete!\r\n");
}
