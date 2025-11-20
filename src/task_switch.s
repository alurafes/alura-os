global task_manager_task_switch

task_manager_task_switch:
    push ebx
    push esi
    push edi
    push ebp

    mov edi, [esp + 20]     ; old
    mov esi, [esp + 24]     ; new

    mov eax, [esi + 4]      ; save new task's stack 
    mov [edi + 4], esp      ; save current stack to old task's esp variable

    mov esp, eax
    mov eax, [esi + 8]
    mov ebx, cr3
    cmp eax, ebx
    je .task_switch_finish
    mov cr3, eax

.task_switch_finish:
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret