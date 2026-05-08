#ifndef ACPI_CPU_H_
#define ACPI_CPU_H_

#define MAXIMUM_CPUS 16384

struct cpu {
    struct limine_mp_info *cpu_info;
};

extern struct cpu cpus[MAXIMUM_CPUS];
extern unsigned int cpu_count;

void register_new_cpu(struct limine_mp_info *cpu);
void prepare_cpus(void);

#endif
