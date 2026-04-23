#include <util/postinit.h>
#include <mm/pmm.h>
#include <terminal.h>
#include <util/random.h>

void postInit(void)
{
    klog(LOG_PROC, "Post-initializing...\r\n");

    init_rand();

    uint64_t free_ram = initPmm();
    klog(LOG_NOTICE, "Usable RAM: %d bytes\r\n", (int64_t)free_ram);

    klog(LOG_SUCCESS, "Post-init complete!\r\n");
}
