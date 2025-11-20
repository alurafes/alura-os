global irq_stub_handler
extern irq_handler
extern task_manager
extern task_manager_task_switch
irq_stub_handler:
    pushad
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call irq_handler
    add esp, 4

    cmp dword [task_manager + 8], 0
    je .irq_stub_handler_normal

    mov dword [task_manager + 8], 0
    jmp task_manager_task_switch
    
.irq_stub_handler_normal
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 8
    iret