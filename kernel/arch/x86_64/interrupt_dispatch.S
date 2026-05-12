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

sti
call handle_interrupt
cli
nop
.endm

.macro ISR vector:req
.global isr\vector
isr\vector :
    cli
    nop

    pushq $0 # All end-of-interrupt code assumes that there is an error code on the top of the stack, or a parameter, so we push an irrelevant value.
    pushq $\vector
    push_registers

    dispatch

    pop_registers
    add $16, %rsp

    sti
    iretq
.endm

.macro ISRERR vector:req
.global isr\vector
isr\vector :
    cli
    nop

    # The error code is handled for us by the CPU.
    pushq $\vector
    push_registers

    dispatch

    pop_registers
    add $16, %rsp

    sti
    iretq
.endm

# OTHER INTERRUPTS
.altmacro

.macro GEN_ISR
    .set CUR_VECTOR, 0
    .rept 256
        ISR %CUR_VECTOR
        .set CUR_VECTOR, CUR_VECTOR + 0x1
    .endr
.endm

GEN_ISR

.section .rodata

.macro GET_ISR_QUAD vec:req
    .quad isr\vec
.endm

.macro GEN_ISR_ARRAY
.align 16
.global isr_array
isr_array:
    .set VEC, 0
    .rept 256
        GET_ISR_QUAD %(VEC)
        .set VEC, VEC + 1
    .endr
.endm

GEN_ISR_ARRAY

.noaltmacro
