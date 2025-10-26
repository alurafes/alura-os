MB_MAGIC equ 0x1BADB002
MB_FLAGS equ 0
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)

section .multiboot
    align 4
    dd MB_MAGIC
    dd MB_FLAGS
    dd MB_CHECKSUM

.bootstrap.stack
align 16
bootstrap_stack_bottom:
    resb 1024
bootstrap_stack_top:

section .start
align 16
global _start
extern setup_bootstrap_paging
extern kernel_main
_start:
    cli

    mov esp, bootstrap_stack_top
    push eax
    push ebx
    
    call setup_bootstrap_paging

    pop ebx
    pop eax

    mov esp, stack_top

    push ebx
    call kernel_main

    hlt

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
