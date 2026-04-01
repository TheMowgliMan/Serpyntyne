.section .text

.global setGdt
.global reloadSegments
.global setTss

setGdt:
    cli
    lgdt (%rdi)
    ret

reloadSegments:
    # Reload CS
    pushq $0x0008
    lea reload_cs(%rip), %rax
    pushq %rax
    lretq
reload_cs:
    # Reload data segments
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ret

setTss:
    ltr %di
    ret
