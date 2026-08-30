global _start
extern main
extern exit

section .text
_start:
    mov eax, [esp]
    lea ebx, [esp + 4]

    push ebx
    push eax
    call main

    add esp, 12

    push eax
    call exit
    add esp, 4

.halt:
    hlt
    jmp .halt