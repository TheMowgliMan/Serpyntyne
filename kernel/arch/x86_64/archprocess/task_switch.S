# Now before me a trial great lays,
# All before it a trifle seeming;
# What look'd an evil river to gaze
# Now a brook of miniscule streaming.
#
# Now I embark on a task of woe,
# As it embarks hardship to inflict;
# A quest to learn Assembly to go
# As I seemeth to be derelict.
#   ---Caleb Reeves (kkidslogin), 5/10/26

# `struct process` member offsets:
.set P_PID, 0
.set P_LOCKED, 4
.set P_LOCK_TARGET, 8
.set P_PROC_MAP, 16
.set P_STATIC_PRIORITY, 24
.set P_DYNAMIC_PRIORITY, 26
.set P_REAL_TIME_PRIORITY, 34
.set P_TICKS_SINCE_LAST_RUN, 38
.set P_DESIRED_RT_INTERVAL, 42
.set P_RSP, 44
.set P_IS_CUR, 52
.set P_NEXT, 56

.section .text
# I know that this (vvv) is unnecessary, but it makes me feel good
.extern tss_addr

# Declared in C as:
#   void switch_task(struct process *prev, struct process *next);
#   `struct process *prev` is in `%rdi`
#   `struct process *next` is in `%rsi`
.global switch_task
switch_task:
    xor %r11, %r11 # We use this register for comparing later

    # First we push the relevant registers
    # (`%rsp` is saved in `struct process`)
    # `%rip` is saved by the callers `call`,
    # and the remainder are caller-saved in SysV
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # Don't do the below if the last process is null
    cmpq %rdi, %r11
    je .new_process_stack

    # Save `%rsp` in `prev->rsp`
    movq %rsp, P_RSP(%rdi)

.new_process_stack:
    # Load the new stack pointer
    movq P_RSP(%rsi), %rsp

    # Set the new RSP0 in the TSS
    movq $tss_addr, %rax
    movq %rsp, 4(%rax)

    # Compare page tables (if same, don't switch)
    cmpq %rdi, %r11
    je .load_next
    movq P_PROC_MAP(%rdi), %rax
    movq P_PROC_MAP(%rsi), %rdx
    cmpq %rax, %rdx
    je .load_next
    movq (%rdx), %rdx
    movq %rdx, %cr3

.load_next:
    # Pop the registers in reverse order
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx

    ret
