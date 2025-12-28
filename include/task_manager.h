#ifndef ALURA_TASK_H
#define ALURA_TASK_H

#include <stdint.h>
#include <stddef.h>

#include "kernel_heap.h"
#include "memory_paging.h"
#include "idt.h"
#include "timer.h"

#define TASK_MANAGER_QUEUE_LEVELS 4
#define TASK_MANAGER_DEFAULT_TIME_SLICE 5
#define TASK_MANAGER_QUEUE_INDEX_BLOCKED TASK_MANAGER_QUEUE_LEVELS
#define TASK_MANAGER_PRIORITY_BOOST_INTERVAL (5 * TIMER_PHASE)

#define TASK_MANAGER_KERNEL_CODE_SELECTOR 0x08
#define TASK_MANAGER_KERNEL_DATA_SELECTOR 0x10

#define TASK_MANAGER_USER_CODE_SELECTOR 0x1B
#define TASK_MANAGER_USER_DATA_SELECTOR 0x23

typedef enum task_manager_result_t {
    TASK_MANAGER_RESULT_OK,
    TASK_MANAGER_RESULT_QUEUE_TASK_NOT_FOUND,
} task_manager_result_t;

typedef enum task_state_t {
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED,
    TASK_STATE_SLEEPING
} task_state_t;

typedef struct task_t {
    uint32_t task_id;
    uint32_t task_esp;
    uint32_t task_cr3;

    task_state_t task_state;
    uint32_t task_time_slice;
    uint32_t task_queue_level;

    uint32_t kernel_stack_base;
    uint32_t kernel_stack_top;

    uint32_t user_stack_base;
    uint32_t user_stack_top;
    uint8_t task_is_user;

    struct task_t* next;
} task_t;

typedef struct task_manager_t {
    task_t* task_current;

    task_t* task_next; // task to switch to
    uint32_t task_needs_switching;

    task_t* task_idle;
    task_t* task_queues[TASK_MANAGER_QUEUE_LEVELS + 1]; // last queue for blocked tasks
    
    uint64_t last_priority_boost_at_ticks;
} task_manager_t;

extern task_manager_t task_manager;
void task_manager_module_init();

task_t* task_manager_task_create(task_manager_t* task_manager, void (*entry)(void), uint8_t task_is_user);
void task_manager_schedule(task_manager_t* task_manager);
task_t* task_manager_create_idle_task(task_manager_t* task_manager);
task_t* task_manager_pick_task(task_manager_t* task_manager);
uint32_t task_manager_calculate_time_slice(task_manager_t* task_manager, uint32_t queue_level);
void task_manager_boost_priority_of_all_tasks(task_manager_t* task_manager);

void task_manager_requeue_task(task_manager_t* task_manager, task_t* task, uint32_t used_time_slice);
void task_manager_enqueue_task(task_manager_t* task_manager, size_t queue_index, task_t* task);
task_t* task_manager_dequeue_task(task_manager_t* task_manager, size_t queue_index);
task_manager_result_t task_manager_remove_task_from_queue(task_manager_t* task_manager, size_t queue_index, task_t* task);

#endif // ALURA_TASK_H