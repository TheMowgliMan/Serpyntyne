#include <archutil/intframe.h>

void print_registers(struct intframe* iframe)
{
    kprintf("RIP: %x RSP: %x\r\n", (int64_t)(iframe->rip), (int64_t)(iframe->rsp));
    kprintf("RFlags: %b\r\n", (int64_t)(iframe->rflags));
    kprintf("RAX: %x RBX: %x RCX: %x RDX: %x\r\n",
            (int64_t)(iframe->rax),
            (int64_t)(iframe->rbx),
            (int64_t)(iframe->rcx),
            (int64_t)(iframe->rdx));
    kprintf("RSI: %x RDI: %x RBP: %x R8: %x\r\n",
            (int64_t)(iframe->rsi),
            (int64_t)(iframe->rdi),
            (int64_t)(iframe->rbp),
            (int64_t)(iframe->r8));
    kprintf("R9: %x R10: %x R11: %x R12: %x\r\n",
            (int64_t)(iframe->r9),
            (int64_t)(iframe->r10),
            (int64_t)(iframe->r11),
            (int64_t)(iframe->r12));
    kprintf("R13: %x R14: %x R15: %x CS: %x\r\n",
            (int64_t)(iframe->r13),
            (int64_t)(iframe->r14),
            (int64_t)(iframe->r15),
            (int64_t)(iframe->cs));
}
