#ifndef ELF_H_
#define ELF_H_

#include <stdint.h>

#define ELF_MAGIC 0x7f454c46

#define ELF_32BIT 1
#define ELF_64BIT 2

#define ELF_ENDIAN_LITTLE 1
#define ELF_ENDIAN_BIT 2

#define ELF_HEADER_VERSION 1
#define ELF_VERSION 1

#define ELF_ARCH_X86_64 0x3e

struct __attribute__((packed)) elf_header_64 {
    char magic[4];
    uint8_t bits;
    uint8_t endian;
    uint8_t header_version;
    uint8_t osabi;
    uint64_t padding;
    uint16_t type;
    uint16_t arch;
    uint32_t elf_version;
    uint64_t program_entry_offset;
    uint64_t program_header_table_offset;
    uint64_t section_header_table_offset;
    uint32_t flags;
    uint16_t elf_header_size;
    uint16_t program_header_entry_size;
    uint16_t program_header_entry_count;
    uint16_t section_header_entry_size;
    uint16_t section_header_entry_count;
    uint16_t section_header_string_table_section_index;
};

#define ELF_PHEADER_SEGMENT_TYPE_NULL 0
#define ELF_PHEADER_SEGMENT_TYPE_LOAD 1
#define ELF_PHEADER_SEGMENT_TYPE_DYNAMIC 2
#define ELF_PHEADER_SEGMENT_TYPE_INTERPRET 3
#define ELF_PHEADER_SEGMENT_TYPE_NOTE 4

#define ELF_PHEADER_FLAGS_EXECUTABLE 1
#define ELF_PHEADER_FLAGS_WRITABLE 2
#define ELF_PHEADER_FLAGS_READABLE 4

struct __attribute__((packed)) elf_program_header_64 {
    uint32_t type_of_segment;
    uint32_t flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesize;
    uint64_t p_memsize;
    uint64_t alignment;
};

#endif
