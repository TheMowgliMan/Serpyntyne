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

extern void setGdt(struct GDTR* gdtr);
extern void reloadSegments(void);

void encodeGDTItem(uint8_t *target, struct GDTItem source);
void initGDT(void);

#endif
