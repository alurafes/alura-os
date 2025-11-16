#include "task_manager.h"

task_manager_t task_manager;

static uint32_t task_id = 0;

task_t* task_manager_task_create(void (*entry)(void))
{
    task_t* task = (task_t*)kernel_heap_calloc(sizeof(task_t));

    task->id = task_id++;

    uint32_t* task_stack = (uint32_t*)kernel_heap_calloc(4096);
    uint32_t* task_stack_top = (uint32_t*)((uint32_t)task_stack + 4096);
    *--task_stack_top = (uint32_t)entry;

    task->esp = (uint32_t)task_stack_top - 4 * sizeof(uint32_t);
    task->cr3 = (uint32_t)virtual_to_physical(page_directory); // for now we put the kernel page directory.

    return task;
}

task_t* task_manager_task_create_idle()
{
    task_t* task = (task_t*)kernel_heap_calloc(sizeof(task_t));

    task->id = task_id++;

    __asm__ volatile("mov %%esp, %0" : "=r"(task->esp));
    __asm__ volatile("mov %%cr3, %0" : "=r"(task->cr3));

    return task;
}

void task_manager_module_init()
{
    task_manager.task_current = task_manager_task_create_idle();
    task_manager.task_queue = NULL;
}