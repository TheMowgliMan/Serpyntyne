#ifndef GDT_H_
#define GDT_H_

#include <stdint.h>

// Allows for simpler accessing
#define DESC GDT_ENTRY_LENGTH *

struct GDTItem
{
    uint32_t base;
    uint32_t limit;

    uint8_t access_byte;
    uint8_t flags;
};

struct __attribute__((packed)) GDTR
{
    uint16_t limit;
    uint64_t base;
};

struct __attribute__((packed)) TSS
{
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist [7];
    uint64_t reserved2;
    uint16_t reserved 3, io_map_base;
}

extern void setGdt(struct GDTR* gdtr);
extern void reloadSegments(void);
extern void setTss(uint16_t tss);

void encodeGDTItem(uint8_t *target, struct GDTItem source);
void initGDT(void);

#endif
