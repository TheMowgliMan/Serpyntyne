#include <acpi/cpu.h>

#include <util/liminereq.h>

#include <memory.h>

#include <stdint.h>

struct cpu cpus[MAXIMUM_CPUS]; // Surely all the CPUs we need
unsigned int cpu_count;

void register_new_cpu(struct limine_mp_info *cpu)
{
    cpus[cpu_count].cpu_info = cpu;
    cpu_count++;
}

void prepare_cpus(void)
{
    klog(LOG_PROC, "Preparing system CPUs...\r\n");

    if (mp_response->cpu_count > MAXIMUM_CPUS)
    {
        klog(LOG_ERROR, "Frick bro why do you have more than sixteen thousand CPUs!?\r\n");
        klog(LOG_NOTICE, TTY_HIMAGENTA "You will only be able to use the first 16384 processors of this system!\r\n");
    }
    klog(LOG_INFO, "This system has %d logical processors.\r\n", mp_response->cpu_count);

    memset(cpus, 0, MAXIMUM_CPUS * sizeof(struct limine_mp_info*));

    while (cpu_count < mp_response->cpu_count)
    {
        /* register_new_cpu() takes care of incrementing `cpu_count` for us */
        register_new_cpu(mp_response->cpus[cpu_count]);
    }

    klog(LOG_SUCCESS, "CPUs prepared!\r\n");
}
