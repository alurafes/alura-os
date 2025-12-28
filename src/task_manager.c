#include "task_manager.h"
#include "print.h"

task_manager_t task_manager;

static uint32_t task_id = 0;

void task_manager_enqueue_task(task_manager_t* task_manager, size_t queue_index, task_t* task)
{
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
    while (head != NULL)
    {
        if (head == task)
        {
            head->next = task->next;
            task->next = NULL;
            return TASK_MANAGER_RESULT_OK;
        }
        head = head->next;
    }
    return TASK_MANAGER_RESULT_QUEUE_TASK_NOT_FOUND;
}

task_t* task_manager_task_create(task_manager_t* task_manager, void (*entry)(void), uint8_t task_is_user)
{
    task_t* task = (task_t*)kernel_heap_calloc(sizeof(task_t)); 

    task->task_id = task_id++;
    task->next = NULL;

    task->task_state = TASK_STATE_READY;
    task->task_time_slice = TASK_MANAGER_DEFAULT_TIME_SLICE;
    task->task_queue_level = 0; // new tasks with the highest queue level

    uint32_t* kernel_stack = (uint32_t*)kernel_heap_calloc(4096);
    task->kernel_stack_top = (uint32_t)kernel_stack + 4096;
    task->kernel_stack_base = (uint32_t)kernel_stack;

    task->task_is_user = task_is_user;
    if (task_is_user)
    {
        uint32_t* user_stack = (uint32_t*)kernel_heap_calloc(4096);
        task->user_stack_top = (uint32_t)user_stack + 4096;
        task->user_stack_base = (uint32_t)user_stack;
    }

    uint32_t* task_stack_top = (uint32_t*)task->kernel_stack_top;

    if (task_is_user)
    {
        *--task_stack_top = TASK_MANAGER_USER_DATA_SELECTOR;
        *--task_stack_top = task->user_stack_top;
    }

    *--task_stack_top = 0x202; // eflags
    *--task_stack_top = task_is_user ? TASK_MANAGER_USER_CODE_SELECTOR : TASK_MANAGER_KERNEL_CODE_SELECTOR; // cs
    *--task_stack_top = (uint32_t)entry; // eip

    *--task_stack_top = 0; // irq number
    *--task_stack_top = 0; // error code
    
    *--task_stack_top = 0; // eax
    *--task_stack_top = 0; // ecx
    *--task_stack_top = 0; // edx
    *--task_stack_top = 0; // ebx
    *--task_stack_top = 0; // esp
    *--task_stack_top = 0; // ebp
    *--task_stack_top = 0; // esi
    *--task_stack_top = 0; // edi

    uint16_t data_selector = task_is_user ? TASK_MANAGER_USER_DATA_SELECTOR : TASK_MANAGER_KERNEL_DATA_SELECTOR;

    *--task_stack_top = data_selector; // ds
    *--task_stack_top = data_selector; // es
    *--task_stack_top = data_selector; // fs
    *--task_stack_top = data_selector; // gs

    task->task_esp = (uint32_t)task_stack_top;
    task->task_cr3 = (uint32_t)virtual_to_physical(page_directory); // for now we put the kernel page directory.

    task_manager_enqueue_task(task_manager, task->task_queue_level, task);

    return task;
}

void task_manager_idle_task()
{
    printf("testing\n");
    while (1) {  
        __asm__ volatile("hlt"); 
    }
}

void task_manager_module_init()
{
    task_manager.task_idle = task_manager_task_create(&task_manager, task_manager_idle_task, 0);
    task_manager.task_idle->task_state = TASK_STATE_RUNNING;
    task_manager.task_current = task_manager.task_idle;
    task_manager.task_next = NULL;
    task_manager.task_needs_switching = 0;
    task_manager.last_priority_boost_at_ticks = timer_get_ticks();
    for (size_t i = 0; i < TASK_MANAGER_QUEUE_LEVELS; ++i)
    {
        task_manager.task_queues[i] = NULL;
    }
}

void task_manager_schedule(task_manager_t* task_manager)
{
    if (timer_get_ticks() - task_manager->last_priority_boost_at_ticks >= TASK_MANAGER_PRIORITY_BOOST_INTERVAL)
    {
        task_manager_boost_priority_of_all_tasks(task_manager);
    }
    if (task_manager->task_current != NULL)
    {
        task_manager->task_current->task_time_slice--;

        if (task_manager->task_current->task_time_slice <= 0)
        {
            if (task_manager->task_current->task_state == TASK_STATE_RUNNING)
            {
                task_manager_requeue_task(task_manager, task_manager->task_current, 1);
            }

            task_t* new_task = task_manager_pick_task(task_manager);
            if (new_task != task_manager->task_current)
            {
                new_task->task_state = TASK_STATE_RUNNING;
                task_manager->task_next = new_task;
                task_manager->task_needs_switching = 1;
            } 
            else
            {
                // prolongating task
                new_task->task_time_slice = task_manager_calculate_time_slice(task_manager, new_task->task_queue_level);
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
            task->task_time_slice = task_manager_calculate_time_slice(task_manager, queue_index);
            return task;
        }
    }

    return task_manager->task_idle;
}

uint32_t task_manager_calculate_time_slice(task_manager_t* task_manager, uint32_t queue_level)
{
    // higher priority tasks get less time to execute so they can switch around quicker
    return TASK_MANAGER_QUEUE_INDEX_BLOCKED * (1 << queue_level);
}

void task_manager_requeue_task(task_manager_t* task_manager, task_t* task, uint32_t used_time_slice)
{
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