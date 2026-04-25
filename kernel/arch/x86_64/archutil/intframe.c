#include <archutil/intframe.h>
#include <terminal.h>
#include <archutil/asmstubs.h>

char *exception_vector_names[32] = {
    "DIVISION_BY_ZERO",
    "DEBUG_EXCEPTION",
    "NON-MASKABLE_INTERRUPT",
    "BREAKPOINT",
    "OVERFLOW",
    "BOUND_RANGE_EXCEEDED",
    "INVALID_OPCODE",
    "DEVICE_NOT_AVAILABLE",
    "DOUBLE_FAULT",
    "COPROCESSOR_SEGMENT_OVERRUN",
    "INVALID_TASK_STATE_SEGMENT",
    "SEGMENT_NOT_PRESENT",
    "STACK-SEGMENT_FAULT",
    "GENERAL_PROTECTION_FAULT",
    "PAGE_FAULT",
    "RESERVED",
    "X87_FLOATING-POINT_EXCEPTION",
    "ALIGNMENT_CHECK",
    "MACHINE_CHECK",
    "SIMD_FLOATING-POINT_EXCEPTION",
    "VIRTUALIZATION_EXCEPTION",
    "CONTROL_PROTECTION_EXCEPTION",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "HYPERVISOR_INJECTION_EXCEPTION",
    "VMM_COMMUNICATION_EXCEPTION",
    "SECURITY_EXCEPTION",
    "RESERVED"
};

char *get_exception_name(uint8_t vector)
{
    return exception_vector_names[vector];
}

void print_registers(struct intframe* iframe)
{
    kprintf("\r\nRIP: %x RSP: %x", (int64_t)(iframe->rip), (int64_t)(iframe->rsp)); pad_with_spaces();
    kprintf("\r\nRFlags: %b", (int64_t)(iframe->rflags)); pad_with_spaces();

    kprintf("\r\nRAX: %x RBX: %x RCX: %x RDX: %x",
            (int64_t)(iframe->rax),
            (int64_t)(iframe->rbx),
            (int64_t)(iframe->rcx),
            (int64_t)(iframe->rdx));
    pad_with_spaces();

    kprintf("\r\nRSI: %x RDI: %x RBP: %x R8: %x",
            (int64_t)(iframe->rsi),
            (int64_t)(iframe->rdi),
            (int64_t)(iframe->rbp),
            (int64_t)(iframe->r8));
    pad_with_spaces();

    kprintf("\r\nR9: %x R10: %x R11: %x R12: %x",
            (int64_t)(iframe->r9),
            (int64_t)(iframe->r10),
            (int64_t)(iframe->r11),
            (int64_t)(iframe->r12));
    pad_with_spaces();

    kprintf("\r\nR13: %x R14: %x R15: %x CS: %x",
            (int64_t)(iframe->r13),
            (int64_t)(iframe->r14),
            (int64_t)(iframe->r15),
            (int64_t)(iframe->cs));
    pad_with_spaces();

    kprintf("\r\nCR2: %x", (int64_t)read_cr2());
    pad_with_spaces();
}
