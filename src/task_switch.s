global task_manager_task_switch

extern task_manager

task_manager_task_switch:
    push ebx
    push esi
    push edi
    push ebp

    mov edi, [task_manager] ; task_manager.current_task
    mov [edi + 4], esp
    mov esi, [esp + 20] ; getting to the passed parameter from C
    mov [task_manager], esi

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