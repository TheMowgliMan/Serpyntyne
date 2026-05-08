#include <stdint.h>

#include <terminal.h>
#include <memory.h>
#include <paging.h>

#include <mm/heap.h>
#include <mm/vmm.h>

#include <uacpi/uacpi.h>
#include <uacpi/acpi.h>
#include <uacpi/tables.h>
#include <uacpi/log.h>
#include <uacpi/context.h>

#include <acpi/acpi.h>
#include <acpi/cpu.h>

#include <util/liminereq.h>
#include <util/align.h>

#include <archutil/asmstubs.h>

#include <stdint.h>

uintptr_t lapic_addr;

void write_lapic_register(size_t reg_offset, uint32_t value)
{
    uint32_t *lapic_register = (uint32_t*)PHYS_TO_HHDM(lapic_addr + reg_offset);
    *lapic_register = value;
}

uint32_t get_lapic_register(size_t reg_offset)
{
    uint32_t *lapic_register = (uint32_t*)PHYS_TO_HHDM(lapic_addr + reg_offset);
    return *lapic_register;
}

void init_acpi(void)
{
    klog(LOG_PROC, "Starting uACPI...\r\n");

    void *uacpi_early_buf = krmalloc(32768);

    uacpi_context_set_log_level(UACPI_LOG_DEBUG);

    uacpi_setup_early_table_access(uacpi_early_buf, 32768);

    if (!uacpi_table_subsystem_available())
        klog(LOG_ERROR, "uACPI table subsytem is unavailable!");

    uacpi_table *madt = krmalloc(16384);
    uacpi_table_find_by_signature(ACPI_MADT_SIGNATURE, madt);

    struct acpi_hdr_madt *madt_hdr = (struct acpi_hdr_madt*)(madt->ptr);
    uint8_t *madt_data = (uint8_t*)kvmalloc(madt_hdr->acpi_hdr.len);
    memcpy(madt_data, madt->ptr, madt_hdr->acpi_hdr.len);

    lapic_addr = madt_hdr->lapic_addr;

    klog(LOG_INFO, "MADT size: %d\r\n", (int64_t)(madt_hdr->acpi_hdr.len));

    for (size_t offset = 0x2c; offset < madt_hdr->acpi_hdr.len; offset += madt_data[offset + 1])
    {
        struct acpi_madt_entry_generic *cur = (struct acpi_madt_entry_generic *)(madt_data + offset);
        klog(LOG_NOTICE, "MADT entry found of type %d\r\n", cur->type);

        switch (cur->type) {
            case ACPI_MADT_ENTRY_LAPIC_ADDR_OVERRIDE:
                klog(LOG_INFO, "LAPIC address override found!\r\n");
                struct acpi_madt_entry_lapic_addr_override *cur_typed_addrovr = (struct acpi_madt_entry_lapic_addr_override *)cur;
                lapic_addr = cur_typed_addrovr->addr;
                break;
            case ACPI_MADT_ENTRY_LAPIC:
                struct acpi_madt_entry_lapic *cur_typed_lapic = (struct acpi_madt_entry_lapic *)cur;
                klog(LOG_INFO, "LAPIC found, processor ID %d, LAPIC ID %d\r\n", (int64_t)cur_typed_lapic->acpi_processor_id, (int64_t)cur_typed_lapic->lapic_id);
                break;
        }
    }

    /* We need to force the LAPIC to be mapped in the HHDM */
    map_page(kernel_page_table, lapic_addr + hhdm_response->offset, gen_frame(lapic_addr, PAGE_SIZE_EXPONENT, false), MAP_NEWMAP | MAP_KERNEL | MAP_READABLE | MAP_WRITABLE | MAP_WRITETHROUGH | MAP_CACHEDISABLE);

    klog(LOG_NOTICE, "LAPIC physical address: %x\r\n", (int64_t)lapic_addr);

#ifdef DEBUG
    klog(LOG_INFO, "Starting APIC on BSP...\r\n");
#endif

    /**
     * A lot of these gymnastics may be unnecessary since Limine already started the LAPIC, but, eh ¯\(ツ)/¯
     * Can't really hurt to ensure state!
     * Plus will have to change this later for more advanced APIC setup anyways
     */

    write_lapic_register(0x0e0, 0xffffffff); // Destination format register
    write_lapic_register(0x0d0, 0x01000000); // Logical destination register

    write_lapic_register(0x320, APIC_LVT_DISABLE); // Disable timer
    write_lapic_register(0x350, APIC_LVT_DISABLE); // Disable LINT0
    write_lapic_register(0x360, APIC_LVT_DISABLE); // Disable LINT1

    write_lapic_register(0x340, APIC_LVT_NMI); // Make PMC an NMI

    write_lapic_register(0x080, 0); // Enable all interrupts in the Task Priority Register

    write_lapic_register(0x0f0, 0x1ff); // Spurious interrupt vector, which software enables the APIC

#ifdef DEBUG
    klog(LOG_INFO, "Starting and calibrating LAPIC Timer...\r\n");
#endif

    write_lapic_register(0x320, 0xfa); // Setting the APIC timer to vector 0xFA also unmasks it and sets it to one-shot mode
    write_lapic_register(0x3e0, 0x2); // APIC timer: divisor of sixteen

    uint64_t old_time = read_arch_time_stamp_counter();
    APIC_TIMER_SET_INITIAL_COUNT(0xFFFFFFFF);

    for (uint64_t i = 0; i < 100000; i++) { asm volatile ("pause"); } // Do a hot nothing for a moment

    write_lapic_register(0x320, APIC_LVT_DISABLE); // Disable timer again

    uint64_t new_time = read_arch_time_stamp_counter();
    uint32_t apic_timer_value = APIC_TIMER_CURRENT_COUNT;

    uint64_t musec_difference = TIME_BETWEEN(old_time, new_time);

#ifdef DEBUG
    klog(LOG_INFO, "Musec difference: %d, APIC timer value: %x\r\n", (int64_t)musec_difference, (int64_t)apic_timer_value);
#endif

    APIC_TIMER_SET_INITIAL_COUNT((0xffffffff - apic_timer_value) * (ALIGN_UP(300000, musec_difference) / musec_difference)); // Get an interrupt every 300ms please

    write_lapic_register(0x320, 0x200fa); // Re-enable APIC timer in periodic mode
    write_lapic_register(0x3e0, 0x2); // Some say you must do the divisor again
}
