#include <util/postinit.h>
#include <mm/pmm.h>
#include <terminal.h>
#include <util/random.h>

void postInit(void)
{
    kprintf(TTY_CYAN "Post-initializing...\r\n" TTY_RESET);

    init_rand();

    uint64_t free_ram = initPmm();
    kprintf(TTY_MAGENTA "Free RAM: %d bytes\r\n" TTY_RESET, (int64_t)free_ram);

    kputs(TTY_GREEN "Post-init complete!\r\n" TTY_RESET);
}
