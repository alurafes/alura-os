#include "task_manager.h"
#include "print.h"
#include "libc/string.h"

task_manager_t task_manager;

static uint32_t task_id = 0;

void task_manager_enqueue_task(task_manager_t* task_manager, size_t queue_index, task_t* task)
{
    if (task == task_manager->task_idle) return;
    task->next = NULL;

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

task_manager_result_t task_manager_prepare_new_stack(page_entry_t* task_page_directory, uint8_t task_is_user, uint32_t eip, uint32_t* out_esp)
{
    void* kernel_stack_phys = memory_bitmap_allocate();

    memory_paging_map(task_page_directory, (uint32_t)kernel_stack_phys, KERNEL_STACK_TOP - PAGE_SIZE, PAGE_READ_WRITE);

    uintptr_t task_stack_base = (uintptr_t)bounce_alloc((uintptr_t)kernel_stack_phys);

    if (task_is_user)
    {
        void* user_stack_phys = memory_bitmap_allocate();
        memory_paging_map(task_page_directory, (uint32_t)user_stack_phys, USER_STACK_TOP - PAGE_SIZE, PAGE_USER | PAGE_READ_WRITE);
    }

    uint32_t* task_stack_top = (uint32_t*)(task_stack_base + PAGE_SIZE);

    if (task_is_user)
    {
        *--task_stack_top = TASK_MANAGER_USER_DATA_SELECTOR;
        *--task_stack_top = USER_STACK_TOP;
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
    task->next = NULL;

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

    task_manager_result_t result = task_manager_prepare_new_stack(task_page_directory, task_is_user, (uint32_t)entry, &task->task_esp);
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

        if (task_manager->task_current->task_time_slice <= 0)
        {
            task_t* old_task = task_manager->task_current;

            if (old_task != task_manager->task_idle &&
                old_task->task_state == TASK_STATE_RUNNING)
            {
                task_manager_requeue_task(
                    task_manager,
                    old_task,
                    1);
            }

            task_t* new_task =
                task_manager_pick_task(task_manager);

            new_task->task_state = TASK_STATE_RUNNING;

            if (new_task != old_task)
            {
                task_manager->task_next = new_task;
                task_manager->task_needs_switching = 1;
            }
            else
            {
                new_task->task_time_slice =
                    task_manager_calculate_time_slice(
                        new_task->task_queue_level);
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
            task->task_time_slice = task_manager_calculate_time_slice(queue_index);
            return task;
        }
    }
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