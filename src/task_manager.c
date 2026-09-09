#include "task_manager.h"
#include "print.h"
#include "libc/string.h"
#include "resourse.h"

task_manager_t task_manager;

static uint32_t task_id = 0;

void task_manager_enqueue_task(task_manager_t* task_manager, size_t queue_index, task_t* task)
{
    if (task == task_manager->task_idle) return;
    task->next = NULL;

    task->task_queue_level = queue_index;

    if (task_manager->task_queues[queue_index] == NULL)
    {
        task_manager->task_queues[queue_index] = task;
        return;
    }

    task_t* head = task_manager->task_queues[queue_index];
    while (head->next != NULL)
    {
        head = head->next;
    }
    head->next = task;
}

task_t* task_manager_dequeue_task(task_manager_t* task_manager, size_t queue_index)
{
    if (task_manager->task_queues[queue_index] == NULL) return NULL;
    task_t* task = task_manager->task_queues[queue_index];
    task_manager->task_queues[queue_index] = task->next;
    task->next = NULL;
    return task;
}

task_manager_result_t task_manager_remove_task_from_queue(task_manager_t* task_manager, size_t queue_index, task_t* task)
{
    if (task_manager->task_queues[queue_index] == NULL) return TASK_MANAGER_RESULT_QUEUE_TASK_NOT_FOUND;
    if (task_manager->task_queues[queue_index] == task)
    {
        task_manager->task_queues[queue_index] = task->next;
        task->next = NULL;
        return TASK_MANAGER_RESULT_OK;
    }
    task_t* head = task_manager->task_queues[queue_index];
    while (head->next != NULL)
    {
        if (head->next == task)
        {
            head->next = task->next;
            task->next = NULL;
            return TASK_MANAGER_RESULT_OK;
        }
        head = head->next;
    }
    return TASK_MANAGER_RESULT_QUEUE_TASK_NOT_FOUND;
}

task_manager_result_t task_manager_prepare_new_stack(page_entry_t* task_page_directory, uint8_t task_is_user, uint32_t eip, uint32_t user_esp, uint32_t* out_esp)
{
    void* kernel_stack_phys = memory_bitmap_allocate();

    memory_paging_map(task_page_directory, (uint32_t)kernel_stack_phys, KERNEL_STACK_TOP - PAGE_SIZE, PAGE_READ_WRITE);

    uintptr_t task_stack_base = (uintptr_t)bounce_alloc((uintptr_t)kernel_stack_phys);
    uint32_t* task_stack_top = (uint32_t*)(task_stack_base + PAGE_SIZE);

    if (task_is_user)
    {
        *--task_stack_top = TASK_MANAGER_USER_DATA_SELECTOR;
        *--task_stack_top = user_esp;
    }

    *--task_stack_top = 0x202; // eflags
    *--task_stack_top = task_is_user ? TASK_MANAGER_USER_CODE_SELECTOR : TASK_MANAGER_KERNEL_CODE_SELECTOR; // cs
    *--task_stack_top = eip;

    *--task_stack_top = 0; // irq number
    *--task_stack_top = 0; // error code

    *--task_stack_top = 0; // eax
    *--task_stack_top = 0; // ecx
    *--task_stack_top = 0; // edx
    *--task_stack_top = 0; // ebx
    *--task_stack_top = 0; // ebp
    *--task_stack_top = 0; // esi
    *--task_stack_top = 0; // edi

    uint16_t data_selector = task_is_user ? TASK_MANAGER_USER_DATA_SELECTOR : TASK_MANAGER_KERNEL_DATA_SELECTOR;

    *--task_stack_top = data_selector; // ds
    *--task_stack_top = data_selector; // es
    *--task_stack_top = data_selector; // fs
    *--task_stack_top = data_selector; // gs

    *out_esp = KERNEL_STACK_TOP - (task_stack_base + PAGE_SIZE - (uint32_t)task_stack_top);
    bounce_free(task_stack_base);

    return TASK_MANAGER_RESULT_OK;
}

task_t* task_manager_task_create(task_manager_t* task_manager, void (*entry)(void), uint8_t task_is_user, uint8_t enqueue)
{
    task_t* task = (task_t*)kernel_heap_calloc(sizeof(task_t)); 

    task->task_id = task_id++;
    task->task_state = TASK_STATE_READY;
    task->task_time_slice = TASK_MANAGER_DEFAULT_TIME_SLICE;
    task->task_queue_level = 0; // new tasks with the highest queue level
    task->task_init_eip = (uint32_t)entry;

    page_entry_t* current_page_directory = (page_entry_t*)PAGE_DIRECTORY_VADDR;
    memory_paging_create_page_directory(&task->task_cr3); // todo: panic!!
    task->stack_base = (uint32_t)KERNEL_STACK_TOP - PAGE_SIZE;
    task->stack_top  = (uint32_t)KERNEL_STACK_TOP;
    task->task_is_user = task_is_user;
    
    page_entry_t* task_page_directory = bounce_alloc(task->task_cr3);

    task_manager_result_t result = task_manager_prepare_new_stack_with_args(task_page_directory, task_is_user, (uint32_t)entry, NULL, &task->task_esp);
    if (result != TASK_MANAGER_RESULT_OK)
    {
        // todo
    }

    bounce_free((uintptr_t)task_page_directory);

    if (enqueue) task_manager_enqueue_task(task_manager, task->task_queue_level, task);

    return task;
}

task_t* task_manager_task_copy(task_manager_t* task_manager, task_t* parent, uint8_t enqueue)
{
    task_t* task = (task_t*)kernel_heap_calloc(sizeof(task_t)); 

    task->task_id = task_id++;
    task->next = NULL;
    task->task_state = TASK_STATE_READY;
    task->task_time_slice = TASK_MANAGER_DEFAULT_TIME_SLICE;
    task->task_queue_level = 0; // new tasks with the highest queue level
    task->task_is_user = parent->task_is_user;
    task->task_init_eip = parent->task_init_eip;
    task->heap_start = parent->heap_start;
    task->heap_break = parent->heap_break;

    memcpy(task->signal_handlers, parent->signal_handlers, sizeof(task->signal_handlers));

    memory_paging_create_page_directory(&task->task_cr3); // todo: panic!!
    page_entry_t* task_page_directory = bounce_alloc(task->task_cr3);

    void* kernel_stack_phys = memory_bitmap_allocate();
    memory_paging_map(task_page_directory, (uint32_t)kernel_stack_phys, KERNEL_STACK_TOP - PAGE_SIZE, PAGE_READ_WRITE);
    task->stack_base = (uint32_t)KERNEL_STACK_TOP - PAGE_SIZE;
    task->stack_top  = (uint32_t)KERNEL_STACK_TOP;

    memory_paging_copy_mapped_memory(task_page_directory); // todo: error checking

    void* kernel_stack = bounce_alloc((uintptr_t)kernel_stack_phys);
    memcpy(kernel_stack, (void*)parent->stack_base, PAGE_SIZE);   

    uint32_t esp_offset = parent->task_esp - parent->stack_base;
    task->task_esp = task->stack_base + esp_offset;

    register_interrupt_data_t* data = (register_interrupt_data_t*)(kernel_stack + esp_offset);
    data->eax = 0;

    bounce_free((uintptr_t)kernel_stack);
    bounce_free((uintptr_t)task_page_directory);

    for (size_t i = 0; i < TASK_MAX_RESOURCES; ++i)
    {
        if (parent->resources[i] != NULL)
        {
            resource_share(task, i, parent->resources[i]);
        }
    }

    if (enqueue) task_manager_enqueue_task(task_manager, task->task_queue_level, task);

    return task;
}

void task_manager_idle_task()
{
    while (1) {
        __asm__ volatile("hlt"); 
    }
}

void task_manager_module_init()
{
    for (size_t i = 0; i < TASK_MANAGER_QUEUE_LEVELS; ++i)
    {
        task_manager.task_queues[i] = NULL;
    }

    task_manager.last_priority_boost_at_ticks = timer_get_ticks();
    task_manager.task_idle = task_manager_task_create(&task_manager, task_manager_idle_task, 0, 0);

    task_manager.task_current = NULL;
    task_manager.task_next = task_manager.task_idle;
    
    task_manager.task_idle->task_state = TASK_STATE_RUNNING;
    task_manager.task_needs_switching = 1;
}

void task_manager_schedule(task_manager_t* task_manager)
{
    memory_paging_destroy_queued();

    if (timer_get_ticks() - task_manager->last_priority_boost_at_ticks >= TASK_MANAGER_PRIORITY_BOOST_INTERVAL)
    {
        task_manager_boost_priority_of_all_tasks(task_manager);
    }

    if (task_manager->task_current != NULL)
    {
        task_manager->task_current->task_time_slice--;

        if (task_manager->task_current->task_time_slice <= 0 || task_manager->task_current->yield)
        {
            task_manager->task_current->yield = 0;
            
            task_t* old_task = task_manager->task_current;

            if (old_task != task_manager->task_idle && old_task->task_state == TASK_STATE_RUNNING)
            {
                task_manager_requeue_task(task_manager, old_task, 1);
            }

            task_t* new_task = task_manager_pick_task(task_manager);

            new_task->task_state = TASK_STATE_RUNNING;

            if (new_task != old_task)
            {
                task_manager->task_next = new_task;
                task_manager->task_needs_switching = 1;
            }
            else
            {
                new_task->task_time_slice = task_manager_calculate_time_slice(new_task->task_queue_level);
            }
        }
    }
}

task_t* task_manager_pick_task(task_manager_t* task_manager)
{
    for (size_t queue_index = 0; queue_index < TASK_MANAGER_QUEUE_LEVELS; ++queue_index)
    {
        if (task_manager->task_queues[queue_index] != NULL)
        {
            task_t* task = task_manager_dequeue_task(task_manager, queue_index);
            if (task->task_state != TASK_STATE_READY || task->task_state != TASK_STATE_RUNNING)
            {
                task->task_time_slice = task_manager_calculate_time_slice(queue_index);
                return task;
            }
        }
    }
    task_manager->task_idle->task_time_slice = task_manager_calculate_time_slice(0);
    return task_manager->task_idle;
}

uint32_t task_manager_calculate_time_slice(uint32_t queue_level)
{
    // higher priority tasks get less time to execute so they can switch around quicker
    return TASK_MANAGER_QUEUE_INDEX_BLOCKED * (1 << queue_level);
}

void task_manager_requeue_task(task_manager_t* task_manager, task_t* task, uint32_t used_time_slice)
{
    if (task == task_manager->task_idle) return;
    if (used_time_slice && task->task_queue_level < TASK_MANAGER_QUEUE_LEVELS - 1)
    {
        task->task_queue_level++;
    }
    task->task_state = TASK_STATE_READY;
    task_manager_enqueue_task(task_manager, task->task_queue_level, task);
}

void task_manager_boost_priority_of_all_tasks(task_manager_t* task_manager)
{
    for (int i = 1; i < TASK_MANAGER_QUEUE_LEVELS; i++)
    {
        while (task_manager->task_queues[i] != NULL)
        {
            task_t* task = task_manager_dequeue_task(task_manager, i);
            task->task_queue_level = 0;
            task_manager_enqueue_task(task_manager, 0, task);
        }
    }
    task_manager->last_priority_boost_at_ticks = timer_get_ticks();
}

task_manager_result_t task_manager_add_child_to_task(task_t* parent, task_t* child)
{
    task_node_t* node = (task_node_t*)kernel_heap_malloc(sizeof(task_node_t));
    node->task = child;
    node->next = NULL;
    node->previous = parent->children_tail;

    if (parent->children == NULL)
    {
        parent->children = node;
        parent->children_tail = node;
        return TASK_MANAGER_RESULT_OK;
    }

    parent->children_tail->next = node;
    parent->children_tail = node;
    return TASK_MANAGER_RESULT_OK;
}

task_manager_result_t task_manager_remove_child_from_task(task_t* parent, task_t* child)
{
    task_node_t* head = parent->children;
    while (head != NULL)
    {
        if (head->task == child)
        {
            if (head->previous != NULL) head->previous->next = head->next;
            else parent->children = head->next;

            if (head->next != NULL) head->next->previous = head->previous;
            else parent->children_tail = head->previous;

            kernel_heap_free(head);
            return TASK_MANAGER_RESULT_OK;
        }
        head = head->next;
    }
    return TASK_MANAGER_RESULT_CHILD_NOT_FOUND;
}

task_manager_result_t task_manager_exit_task(task_manager_t* task_manager, task_t* task, int32_t return_code)
{
    if (task->task_state == TASK_STATE_READY)
    {
        task_manager_remove_task_from_queue(task_manager, task->task_queue_level, task);
    }
    else if (task->task_state == TASK_STATE_BLOCKED)
    {
        task_manager_remove_task_from_queue(task_manager, TASK_MANAGER_QUEUE_INDEX_BLOCKED, task);
    }

    task->task_state = TASK_STATE_ZOMBIE;
    task->return_code = return_code;

    task_node_t* head = task->children;
    while (head != NULL)
    {
        task_node_t* next = head->next;
        task_t *child = head->task;

        child->parent = task_manager->task_init;

        task_node_t *node = kernel_heap_malloc(sizeof(task_node_t));

        if (node == NULL)
        {
            // todo: hmm
            return TASK_MANAGER_RESULT_OUT_OF_MEMORY;
        }

        node->task = child;
        node->next = task_manager->task_init->children;
        task_manager->task_init->children = node;

        kernel_heap_free(head);

        head = next;
    }

    task->children = NULL;

    task_t* parent = task->parent;
    if (
        parent != NULL &&
        parent->task_state == TASK_STATE_BLOCKED &&
        parent->wait_reason == TASK_WAIT_REASON_CHILD &&
        (parent->wait_object == (void*)task->task_id ||
        parent->wait_object == (void*)-1)
    )
    {
        task_manager_unblock_task(task_manager, parent);
    }

    task_manager_enqueue_task(task_manager, TASK_MANAGER_QUEUE_INDEX_ZOMBIE, task);

    return TASK_MANAGER_RESULT_OK;
}

task_manager_result_t task_manager_block_task(task_manager_t* task_manager, task_t* task, task_wait_reason_t wait_reason, void* wait_object)
{
    if (task->task_state == TASK_STATE_BLOCKED) return TASK_MANAGER_RESULT_OK;

    task->task_state = TASK_STATE_BLOCKED;
    task->wait_object = wait_object;
    task->wait_reason = wait_reason;
    task_manager_remove_task_from_queue(task_manager, task->task_queue_level, task);
    task_manager_enqueue_task(task_manager, TASK_MANAGER_QUEUE_INDEX_BLOCKED, task);

    return TASK_MANAGER_RESULT_OK;
}

task_manager_result_t task_manager_unblock_task(task_manager_t* task_manager, task_t* task)
{
    if (task->task_state != TASK_STATE_BLOCKED) return TASK_MANAGER_RESULT_OK;

    task->task_state = TASK_STATE_READY;
    task_manager_remove_task_from_queue(task_manager, task->task_queue_level, task);
    task_manager_enqueue_task(task_manager, 0, task);

    return TASK_MANAGER_RESULT_OK;
}

void task_manager_wake_blocked_on(task_manager_t* task_manager, task_wait_reason_t wait_reason, void* wait_object)
{
    task_t* head = task_manager->task_queues[TASK_MANAGER_QUEUE_INDEX_BLOCKED];
    while (head != NULL)
    {
        task_t* next = head->next;
        if (head->wait_reason == wait_reason && head->wait_object == wait_object)
        {
            task_manager_unblock_task(task_manager, head);
        }
        head = next;
    }
}

task_manager_result_t task_manager_yield_current(task_manager_t* task_manager)
{
    if (task_manager->task_current != NULL)
    {
        task_manager->task_current->yield = 1;
        task_manager_schedule(task_manager);
    }
    return TASK_MANAGER_RESULT_OK;
}

task_t* task_manager_find_child(task_manager_t* task_manager, task_t* parent, uint32_t pid)
{
    task_node_t* head = parent->children;

    while (head != NULL)
    {
        if (head->task->task_id == pid) return head->task;
        head = head->next;
    }

    return NULL;
}

task_t* task_manager_find_zombie_child(task_manager_t* task_manager, task_t* parent)
{
    task_node_t* head = parent->children;

    while (head != NULL)
    {
        if (head->task->task_state == TASK_STATE_ZOMBIE) return head->task;
        head = head->next;
    }

    return NULL;
}

static task_t* task_manager_find_task_recursive(task_t* root, uint32_t pid)
{
    if (root->task_id == pid) return root;

    task_node_t* head = root->children;
    while (head != NULL)
    {
        task_t* found = task_manager_find_task_recursive(head->task, pid);
        if (found != NULL) return found;
        head = head->next;
    }

    return NULL;
}

task_t* task_manager_find_task(task_manager_t* task_manager, uint32_t pid)
{
    return task_manager_find_task_recursive(task_manager->task_init, pid);
}

static void task_manager_copy_memory(page_entry_t* target_cr3_phys, uintptr_t target_vaddr, void* buffer, size_t length, uint8_t to_target)
{
    uint8_t* buf = (uint8_t*)buffer;
    size_t done = 0;

    while (done < length)
    {
        uintptr_t physical = memory_paging_virtual_to_physical(target_cr3_phys, target_vaddr + done);
        if (physical == 0) return;

        uintptr_t page_physical = ALIGN_DOWN(physical);
        uintptr_t page_offset = physical - page_physical;

        size_t chunk = PAGE_SIZE - page_offset;
        if (chunk > length - done) chunk = length - done;

        uint8_t* mapped = (uint8_t*)bounce_alloc(page_physical);
        if (to_target) memcpy(mapped + page_offset, buf + done, chunk);
        else memcpy(buf + done, mapped + page_offset, chunk);
        bounce_free((uintptr_t)mapped);

        done += chunk;
    }
}

static uint8_t task_manager_user_range_mapped(page_entry_t* target_cr3_phys, uintptr_t vaddr, size_t length)
{
    uintptr_t start = ALIGN_DOWN(vaddr);
    uintptr_t end = ALIGN_UP(vaddr + length);

    for (uintptr_t page = start; page < end; page += PAGE_SIZE)
    {
        // memory_paging_virtual_to_physical returns 0 if not present
        if (memory_paging_virtual_to_physical(target_cr3_phys, page) == 0) return 0;
    }

    return 1;
}

void task_manager_deliver_signal(task_t* target, int32_t sig, void (*handler)(int))
{
    uint8_t trampoline[9] = {
        0xB8, // load into eax
        (uint8_t)(SYSCALL_SIGRETURN),
        (uint8_t)(SYSCALL_SIGRETURN >> 8),
        (uint8_t)(SYSCALL_SIGRETURN >> 16),
        (uint8_t)(SYSCALL_SIGRETURN >> 24),
        0xCD, 0x80, // int 0x80
        0xEB, 0xFE  // jmp $ never reached
    };
    uint32_t frame_size = sizeof(trampoline) + 8; // + sig + return address

    uint8_t is_self = (target == task_manager.task_current);
    page_entry_t* target_cr3 = (page_entry_t*)target->task_cr3;

    register_interrupt_data_t frame;
    if (is_self) frame = *syscall.current_register_data;
    else task_manager_copy_memory(target_cr3, (uintptr_t)target->task_esp, &frame, sizeof(frame), 0);

    uint32_t trampoline_addr = frame.useresp - sizeof(trampoline);
    uint32_t arg_addr = trampoline_addr - 4;
    uint32_t retaddr_addr = arg_addr - 4;

    if (frame.useresp < frame_size || !task_manager_user_range_mapped(target_cr3, retaddr_addr, frame_size))
    {
        // screw the task if the stack is filled with crap 
        task_manager_exit_task(&task_manager, target, sig & 0x7f);
        if (is_self) task_manager_yield_current(&task_manager);
        return;
    }

    target->saved_signal_frame = frame;
    target->in_signal_handler = 1;
    target->saved_syscall_retry = target->syscall_retry;
    target->syscall_retry = 0;

    frame.useresp = retaddr_addr;
    frame.eip = (uint32_t)handler;

    if (is_self)
    {
        memcpy((void*)trampoline_addr, trampoline, sizeof(trampoline));
        *(int32_t*)arg_addr = sig;
        *(uint32_t*)retaddr_addr = trampoline_addr;

        *syscall.current_register_data = frame;
        return;
    }

    task_manager_copy_memory(target_cr3, trampoline_addr, trampoline, sizeof(trampoline), 1);
    task_manager_copy_memory(target_cr3, arg_addr, &sig, sizeof(sig), 1);
    task_manager_copy_memory(target_cr3, retaddr_addr, &trampoline_addr, sizeof(trampoline_addr), 1);

    task_manager_copy_memory(target_cr3, (uintptr_t)target->task_esp, &frame, sizeof(frame), 1);
}

void task_manager_destroy_task(task_manager_t* task_manager, task_t* task)
{
    if (task->parent != NULL)
    {
        task->parent->child_ticks_user += task->ticks_user + task->child_ticks_user;
        task->parent->child_ticks_system += task->ticks_system + task->child_ticks_system;
    }

    for (size_t i = 0; i < TASK_MAX_RESOURCES; ++i)
    {
        resource_t* resource = task->resources[i];
        if (!resource) continue;
        resource_remove(task, i);
    }

    if (task->parent != NULL)
    {
        task_manager_remove_child_from_task(task->parent, task);
    }

    task_manager_remove_task_from_queue(task_manager, task->task_queue_level, task);

    page_entry_t* task_page_directory_phys = (page_entry_t*)task->task_cr3;
    if (task_page_directory_phys != kernel_page_directory_phys)
    {
        page_entry_t* task_page_directory = (page_entry_t*)bounce_alloc((uintptr_t)task_page_directory_phys);
        memory_paging_free_page_directory(task_page_directory);
        bounce_free((uintptr_t)task_page_directory);
        memory_bitmap_free(task_page_directory_phys);
    }

    kernel_heap_free(task);
}

uint32_t task_manager_count_arguments(char* const args[])
{
    if (!args) return 0;
    uint32_t count = 0;
    while (args[count] != NULL) count++;
    return count;
}

void* task_manager_map_user_stack(page_entry_t* task_page_directory)
{
    void* user_stack_phys = memory_bitmap_allocate();
    if (!user_stack_phys) return NULL;
    memory_paging_map(task_page_directory, (uint32_t)user_stack_phys, USER_STACK_TOP - PAGE_SIZE, PAGE_USER | PAGE_READ_WRITE);
    return user_stack_phys;
}


task_manager_result_t task_manager_build_arguments_on_stack(void* user_stack_phys, char* const argv[], uint32_t* out_user_esp)
{
    uint32_t argc = task_manager_count_arguments(argv);
    uint32_t argv_addresses[TASK_MAX_ARGUMENTS];

    uint8_t* page = (uint8_t*)bounce_alloc((uintptr_t)user_stack_phys);
    uint32_t page_base_user = USER_STACK_TOP - PAGE_SIZE;
    uint8_t* cursor = page + PAGE_SIZE;

    for (int32_t i = (int32_t)argc - 1; i >= 0; i--)
    {
        size_t len = strlen(argv[i]) + 1;
        cursor -= len;
        memcpy(cursor, argv[i], len);
        argv_addresses[i] = page_base_user + (uint32_t)(cursor - page);
    }

    cursor = (uint8_t*)((uintptr_t)cursor & ~0x3u); // string can be any length, gotta align
    uint32_t needed = 4 + (argc + 1) * 4;
    if ((uint32_t)(cursor - page) < needed)
    {
        bounce_free((uintptr_t)page);
        return TASK_MANAGER_RESULT_ARGS_TOO_LARGE;
    }
    
    uint32_t* word_cursor = (uint32_t*)cursor;
    *--word_cursor = 0; // argv NULL terminator
    for (int32_t i = (int32_t)argc - 1; i >= 0; --i) *--word_cursor = argv_addresses[i];
    *--word_cursor = argc;

    *out_user_esp = page_base_user + (uint32_t)((uint8_t*)word_cursor - page);

    bounce_free((uintptr_t)page);
    return TASK_MANAGER_RESULT_OK;
}

task_manager_result_t task_manager_prepare_new_stack_with_args(page_entry_t* task_page_directory, uint8_t task_is_user, uint32_t eip, char* const argv[], uint32_t* out_esp)
{
    uint32_t user_esp = USER_STACK_TOP;

    if (task_is_user)
    {
        void* user_stack_phys = task_manager_map_user_stack(task_page_directory);
        if (!user_stack_phys) return TASK_MANAGER_RESULT_OUT_OF_MEMORY;

        task_manager_result_t args_result = task_manager_build_arguments_on_stack(user_stack_phys, argv, &user_esp);
        if (args_result != TASK_MANAGER_RESULT_OK) return args_result;
    }

    return task_manager_prepare_new_stack(task_page_directory, task_is_user, eip, user_esp, out_esp);
}