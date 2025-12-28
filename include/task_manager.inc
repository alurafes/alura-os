struc task_manager_t
    .task_current resd 1
    .task_next resd 1
    .task_needs_switching resd 1
endstruc

struc task_t
    .task_id resd 1
    .task_esp resd 1
    .task_cr3 resd 1
    .task_state resd 1
    .task_time_slice resd 1
    .task_queue_level resd 1
    .kernel_stack_base resd 1
    .kernel_stack_top resd 1
    .user_stack_base resd 1
    .user_stack_top resd 1
    .task_is_user resb 1
    .task_next resd 1
endstruc
