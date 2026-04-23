#include <stdint.h>

#include <idt.h>
#include <interrupts.h>
#include <terminal.h>

static struct IDTItem IDT[256];
static struct IDTR idtr;


// Type-copied from 'set_idt_entry()' @ https://git.evalyngoemer.com/evalynOS/evalynOS-old/src/branch/dev/kernel/src/drivers/x86_64/idt.c
void generateIDTEntry(int vector, uint8_t ist, uint8_t attributes, void(*handler)())
{
  uint64_t address = (uint64_t)handler;

  IDT[vector] = (struct IDTItem) {
	.offset1 = address & 0xFFFF,
	.selector = 0x08,
	.ist = ist,
	.type_attributes = attributes,
	.offset2 = (address >> 16) & 0xFFFF,
	.offset3 = (address >> 32),
	.zero = 0
  };
}

void initIDT()
{
  klog(LOG_INFO, "Loading IDT...");

  for (int i = 0x00; i < 256; i++) {
	generateIDTEntry(i, 1, 0x8E, (void (*)())isr0x09);
  }

  generateIDTEntry(0x00, 1, 0x8e, (void (*)())isr0x00);
  generateIDTEntry(0x06, 1, 0x8e, (void (*)())isr0x06);
  generateIDTEntry(0x08, 0, 0x8e, (void (*)())isr0x08);
  generateIDTEntry(0x0d, 1, 0x8e, (void (*)())isr0x0D);
  generateIDTEntry(0x0e, 1, 0x8e, (void (*)())isr0x0E);

  idtr.limit = sizeof(IDT) - 1;
  idtr.base = (uint64_t)&IDT;

  asm volatile ("lidt %0" : : "m"(idtr));
  asm volatile ("sti");

  kputs("done!\r\n");
}
