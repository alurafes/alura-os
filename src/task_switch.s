global task_manager_task_switch

task_manager_task_switch:
    push ebx
    push esi
    push edi
    push ebp

    mov edi, [esp + 4 * (4 + 1)] ; task_manager->current_task
    mov [edi + 4], esp
    mov esi, [esp + 4 * (4 + 2)] ; new task_t
    mov [edi], esi

    mov esp, [esi + 4]
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