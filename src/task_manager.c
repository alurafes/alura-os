#include "task_manager.h"
#include "print.h"

task_manager_t task_manager;

static uint32_t task_id = 0;

void task_manager_queue_task(task_manager_t* task_manager, task_t* task)
{
    if (task_manager->task_queue == NULL)
    {
        task_manager->task_queue = task;
        task->next_task = task;
    }
    else 
    {
        task->next_task = task_manager->task_queue->next_task;
        task_manager->task_queue->next_task = task;
    }
}

task_t* task_manager_task_create(task_manager_t* task_manager, void (*entry)(void))
{
    task_t* task = (task_t*)kernel_heap_calloc(sizeof(task_t));

    task->id = task_id++;

    uint32_t* task_stack = (uint32_t*)kernel_heap_calloc(4096);
    uint32_t* task_stack_top = (uint32_t*)((uint32_t)task_stack + 4096);
    *--task_stack_top = (uint32_t)entry;

    task->esp = (uint32_t)task_stack_top - 4 * sizeof(uint32_t);
    task->cr3 = (uint32_t)virtual_to_physical(page_directory); // for now we put the kernel page directory.

    task_manager_queue_task(task_manager, task);

    return task;
}

void task_manager_task_idle_entry()
{
    while (1) {  
        printf("IDLE\n");
    }
}

void task_manager_module_init()
{
    task_manager.task_queue = NULL;
    task_manager.task_current = task_manager_task_create(&task_manager, task_manager_task_idle_entry);
}

void task_manager_schedule(task_manager_t* task_manager)
{
    if (task_manager->task_current == NULL) return;
    task_manager_task_switch(task_manager, task_manager->task_current->next_task);
}