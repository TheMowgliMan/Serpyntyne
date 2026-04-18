#include <util/postinit.h>
#include <mm/pmm.h>
#include <terminal.h>
#include <util/random.h>

void postInit(void)
{
    kprintf("Post-initializing...\r\n");

    init_rand();

    uint64_t free_ram = initPmm();
    kprintf("Free RAM: %d bytes\r\n", (int64_t)free_ram);

    kputs("\033[0;32mPost-init complete!\033[0;37m\r\n");
}
