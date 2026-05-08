#ifndef ACPI_H_
#define ACPI_H_

#include <stdint.h>

#define PACKED __attribute__((packed))

struct PACKED acpi_hdr_generic {
    char signature[4];
    uint32_t len;
    uint8_t rev;
    uint8_t checksum;
    char oem_id[6];
    uint64_t oem_table_id;
    uint32_t oem_rev;
    uint32_t creator_id;
    uint32_t creator_rev;
};

struct PACKED acpi_hdr_madt {
    struct acpi_hdr_generic acpi_hdr;
    uint32_t lapic_addr;
    uint32_t flags;
};

struct PACKED acpi_madt_entry_generic {
    uint8_t type;
    uint8_t len;
};

#define ACPI_MADT_ENTRY_LAPIC 0
#define ACPI_MADT_ENTRY_IOAPIC 1
#define ACPI_MADT_ENTRY_IOAPIC_SOURCE_OVERRIDE 2
#define ACPI_MADT_ENTRY_IOAPIC_NMI_SOURCE 3
#define ACPI_MADT_ENTRY_LAPIC_NMI 4
#define ACPI_MADT_ENTRY_LAPIC_ADDR_OVERRIDE 5
#define ACPI_MADT_ENTRY_PROC_LOCAL_X2APIC 9

struct PACKED acpi_madt_entry_lapic_addr_override {
    struct acpi_madt_entry_generic hdr;
    uint16_t reserved;
    uint64_t addr;
};

struct PACKED acpi_madt_entry_lapic {
    struct acpi_madt_entry_generic hdr;
    uint8_t acpi_processor_id;
    uint8_t lapic_id;
};

void init_acpi(void);

#define APIC_LVT_DISABLE 0x10000
#define APIC_LVT_NMI (4 << 8)

#define APIC_TIMER_SET_INITIAL_COUNT(X) write_lapic_register(0x380, X)
#define APIC_TIMER_CURRENT_COUNT get_lapic_register(0x390)

void write_lapic_register(size_t reg_offset, uint32_t value);
uint32_t get_lapic_register(size_t reg_offset);

#define APIC_EOI() write_lapic_register(0x0b0, 0)

#endif
