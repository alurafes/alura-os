global task_manager_task_switch

extern task_manager

task_manager_task_switch:
    mov edi, [task_manager]
    mov esi, [edi + 12]

    mov eax, [esi + 4]      ; save new task's stack 
    mov [edi + 4], esp      ; save current stack to old task's esp variable

    mov [task_manager], esi

    mov esp, eax

    mov eax, [esi + 8]
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