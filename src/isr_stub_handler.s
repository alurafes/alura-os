%include "task_manager.inc"

global isr_stub_handler
extern isr_handler
extern task_manager
extern task_manager_task_switch

isr_stub_handler:
    push eax
    push ecx
    push edx
    push ebx
    push ebp
    push esi
    push edi
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

    mov edi, [task_manager + task_manager_t.task_current]
    mov eax, [edi + task_t.task_cr3]
    mov ebx, cr3
    cmp eax, ebx
    je .isr_stub_handler_finish
    mov cr3, eax

.isr_stub_handler_finish:
    cmp dword [task_manager + task_manager_t.task_needs_switching], 0
    je .isr_stub_handler_finish_normal

    mov dword [task_manager + task_manager_t.task_needs_switching], 0
    jmp task_manager_task_switch

.isr_stub_handler_finish_normal:
    pop gs
    pop fs
    pop es
    pop ds
    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax 
    add esp, 8 ; get rid of error code and isr index
    iret