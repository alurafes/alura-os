BITS 32

global _start

section .text

_start:
.loop:
    mov eax, 4          ; syscall number
    mov ebx, msg        ; pointer to string
    int 0x80
    jmp .loop

section .rodata

msg db "Hello from ASM!", 0