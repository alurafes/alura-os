%include "task_manager.inc"
%include "tss.inc"
global task_manager_task_switch

extern task_manager
extern tss

task_manager_task_switch:
    mov edi, [task_manager + task_manager_t.task_current]
    mov esi, [task_manager + task_manager_t.task_next]

    mov [edi + task_t.task_esp], esp      ; save current stack to old task's esp variable
    
    mov eax, [esi + task_t.kernel_stack_top]      ; save new task's stack 
    mov [tss + tss_entry_t.esp0], eax

    mov [task_manager + task_manager_t.task_current], esi

    mov esp, [esi + task_t.task_esp]

    mov eax, [esi + task_t.task_cr3]
    mov ebx, cr3
    cmp eax, ebx
    je .task_switch_finish
    mov cr3, eax

.task_switch_finish:
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 8
    iret