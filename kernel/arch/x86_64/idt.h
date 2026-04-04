#ifndef IDT_H_
#define IDT_H_

#include <stdint.h>

struct __attribute__((packed)) IDTItem
{
  uint16_t offset1;
  uint16_t selector;
  uint8_t ist;
  uint8_t type_attributes;
  uint16_t offset2;
  uint32_t offset3;
  uint32_t zero;
};

struct __attribute__((packed)) IDTR
{
  uint16_t limit;
  uint64_t base;
};

void generateIDTEntry(int vector, uint8_t ist, uint8_t attributes, void (*handler)());
void initIDT();

#endif
