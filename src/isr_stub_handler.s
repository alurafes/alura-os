global isr_stub_handler
extern isr_handler
isr_stub_handler:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10 ; kernel data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call isr_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8 ; get rid of error code and isr index
    iret