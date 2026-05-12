#include <util/postinit.h>

#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>

#include <terminal.h>
#include <memory.h>

#include <util/random.h>

#include <acpi/acpi.h>
#include <acpi/cpu.h>

#include <process/scheduler.h>
#include <process/process_data.h>

extern scheduler_t *bsp_sched_ctx;

uint64_t new_stack[8192];

void postInit(void)
{
    klog(LOG_PROC, "Post-initializing...\r\n");

    init_rand();

    uint64_t free_ram = initPmm();
    klog(LOG_NOTICE, "Usable RAM: %d bytes\r\n", (int64_t)free_ram);

    paging_init();

    heap_init();

    prepare_cpus();

    bsp_sched_ctx = (scheduler_t*)kvmalloc(sizeof(scheduler_t));
    init_scheduler(bsp_sched_ctx);
    scheduler_add_procedure(bsp_sched_ctx, test_proc1, 4096);
    scheduler_add_procedure(bsp_sched_ctx, test_proc2, 4096);

    init_acpi();

    klog(LOG_SUCCESS, "Post-init complete!\r\n");
}
