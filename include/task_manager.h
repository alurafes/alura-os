#ifndef ALURA_TASK_H
#define ALURA_TASK_H

#include <stdint.h>
#include <stddef.h>

#include "kernel_heap.h"
#include "memory_paging.h"

typedef struct task_t {
    uint32_t id;
    uint32_t esp;
    uint32_t cr3;
    struct task_t* next_task;
} task_t;

typedef struct task_manager_t {
    task_t* task_current;
    task_t* task_queue;
} task_manager_t;

extern task_manager_t task_manager;
void task_manager_module_init();

task_t* task_manager_task_create(void (*entry)(void));
extern void task_manager_task_switch(task_t* task);

#endif // ALURA_TASK_H