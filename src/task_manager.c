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

    *--task_stack_top = 0x202; // eflags
    *--task_stack_top = 0x08; // cs
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

    *--task_stack_top = 0x10; // ds
    *--task_stack_top = 0x10; // es
    *--task_stack_top = 0x10; // fs
    *--task_stack_top = 0x10; // gs

    task->esp = (uint32_t)task_stack_top;
    task->cr3 = (uint32_t)virtual_to_physical(page_directory); // for now we put the kernel page directory.

    task_manager_queue_task(task_manager, task);

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
    task_manager.task_queue = NULL;
    task_manager.task_current = task_manager_task_create(&task_manager, task_manager_idle_task);
    task_manager.task_needs_switching = 0;
}

void task_manager_schedule(task_manager_t* task_manager)
{
    task_manager->task_needs_switching = 1;
}