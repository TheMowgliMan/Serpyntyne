# A lot of the code in this file is based on EvalynOS, which is open source.
# The source I used as a reference can be found here: https://git.evalyngoemer.com/evalynOS/evalynOS-old/src/branch/dev/kernel/src/interupts/interupts.asm
# While this is not an exact copy of that software, the implementation design is very similar (one could say that this software is paraphrased), so I decided to cite it here.

.section .text

.extern handle_interrupt

.macro push_registers
pushq %rax
pushq %rbx
pushq %rcx
pushq %rdx
pushq %rsi
pushq %rdi
pushq %rbp
pushq %r8
pushq %r9
pushq %r10
pushq %r11
pushq %r12
pushq %r13
pushq %r14
pushq %r15
.endm

.macro pop_registers
popq %r15
popq %r14
popq %r13
popq %r12
popq %r11
popq %r10
popq %r9
popq %r8
popq %rbp
popq %rdi
popq %rsi
popq %rdx
popq %rcx
popq %rbx
popq %rax
.endm

.macro dispatch
cld
movq %rsp, %rdi

call handle_interrupt
cli
.endm

.macro ISR vector:req
.global isr\vector
isr\vector :
    pushq $0 # All end-of-interrupt code assumes that there is an error code on the top of the stack, or a parameter, so we push an irrelevant value.
    pushq $\vector
    push_registers

    dispatch

    pop_registers
    add $16, %rsp
    iretq
.endm

.macro ISRERR vector:req
.global isr\vector
isr\vector :
    # The error code is handled for us by the CPU.
    pushq $\vector
    push_registers

    dispatch

    pop_registers
    add $16, %rsp

    sti
    iretq
.endm

# EXCEPTIONS
ISR 0x00 # Division Error
ISR 0x01 # Debug
ISR 0x02 # NMI
ISR 0x03 # Breakpoint
ISR 0x04 # Overflow
ISR 0x05 # Bound Range Exceeded
ISR 0x06 # Invalid Opcode
ISR 0x07 # Device not available
ISRERR 0x08 # Double Fault
ISR 0x09 # Coprocessor Segment Overrun (used as an unhandled interrupt handler)
ISRERR 0x0A # Invalid TSS
ISRERR 0x0B # Segment not Present
ISRERR 0x0C # # Stack-Segment Fault
ISRERR 0x0D # General Protection Fault
ISRERR 0x0E # Page Fault
ISR 0x0F # Reserved
ISR 0x10 # x87 FP Exception
ISRERR 0x11 # Alignment Check
ISR 0x12 # Machine Check
ISR 0x13 # SIMD FP Exception
ISR 0x14 # Virtualization Exception
ISRERR 0x15 # Control-Protection Exception
ISR 0x1C # Hypervisor Injection Exception
ISRERR 0x1D # VMM Communication Exception
ISRERR 0x1E # Security Exception
