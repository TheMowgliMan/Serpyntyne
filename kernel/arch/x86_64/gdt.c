#include <terminal.h> // Shut up Flycheck
#include <gdt.h>
#include <limine.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
  .id = LIMINE_HHDM_REQUEST_ID,
  .revision = 0
};

const uint8_t GDT_ENTRIES = 5; // Null, kernel code, kernel data
const uint8_t GDT_ENTRY_LENGTH = 8; // Bytes
const uint8_t GDT_TOTAL_LENGTH = 40;

static uint8_t GDT[40];
static struct GDTR rr;

extern void setGdt(struct GDTR* gdtr);
extern void reloadSegments(void);

struct GDTItem generateGDTItem(uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t flags)
{
  struct GDTItem tmp;
  tmp.base = base;
  tmp.limit = limit;
  tmp.access_byte = access_byte;
  tmp.flags = flags;
  return tmp;
}

// Not quite yoinked from the OSDev Wiki
void encodeGDTItem(uint8_t *target, struct GDTItem source)
{
  if (source.limit > 0xFFFFF) {kerror("GDT Encode failed: limit greater than 0xFFFFF!\r\n"); return;}

  // Limit
  target[0] = source.limit & 0xFF;
  target[1] = (source.limit >> 8) & 0xFF;
  target[6] = (source.limit >> 16) & 0x0F;

  // Base
  target[2] = source.base & 0xFF;
  target[3] = (source.base >> 8) & 0xFF;
  target[4] = (source.base >> 16) & 0xFF;
  target[7] = (source.base >> 24) & 0xFF;

  // Access byte
  target[5] = source.access_byte;

  // Flags
  target[6] |= (source.flags << 4);
}

void initGDT(void)
{
  kputs("Loading GDT...");
  
  struct GDTItem nulld = generateGDTItem(0, 0, 0x00, 0x0);
  encodeGDTItem(&GDT[DESC 0], nulld);

  struct GDTItem kerncsd = generateGDTItem(0, 0xFFFFF, 0x9B, 0xA);
  encodeGDTItem(&GDT[DESC 1], kerncsd);

  struct GDTItem kerndsd = generateGDTItem(0, 0xFFFFF, 0x93, 0xC);
  encodeGDTItem(&GDT[DESC 2], kerndsd);

  struct GDTItem usercsd = generateGDTItem(0, 0xFFFFF, 0xFB, 0xA);
  encodeGDTItem(&GDT[DESC 3], usercsd);

  struct GDTItem userdsd = generateGDTItem(0, 0xFFFFF, 0xF3, 0xC);
  encodeGDTItem(&GDT[DESC 4], userdsd);
  
  rr.limit = (sizeof(uint8_t) * GDT_TOTAL_LENGTH) - 1;
  rr.base = (uint64_t)&GDT;

  setGdt(&rr);
  reloadSegments();

  kputs("success!\r\n");
}
