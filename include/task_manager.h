#ifndef ALURA_TASK_H
#define ALURA_TASK_H

#include <stdint.h>
#include <stddef.h>

#include "kernel_heap.h"
#include "memory_paging.h"
#include "idt.h"
#include "drivers/timer.h"
#include "syscall.h"

typedef struct resource_t resource_t;

#define TASK_MANAGER_QUEUE_LEVELS 4
#define TASK_MANAGER_DEFAULT_TIME_SLICE 5
#define TASK_MANAGER_QUEUE_INDEX_BLOCKED TASK_MANAGER_QUEUE_LEVELS
#define TASK_MANAGER_QUEUE_INDEX_ZOMBIE (TASK_MANAGER_QUEUE_LEVELS + 1)
#define TASK_MANAGER_PRIORITY_BOOST_INTERVAL (5 * TIMER_PHASE)

#define TASK_MANAGER_KERNEL_CODE_SELECTOR 0x08
#define TASK_MANAGER_KERNEL_DATA_SELECTOR 0x10

#define TASK_MANAGER_USER_CODE_SELECTOR 0x1B
#define TASK_MANAGER_USER_DATA_SELECTOR 0x23

#define TASK_MAX_RESOURCES 512

typedef enum task_manager_result_t {
    TASK_MANAGER_RESULT_OK = 0,
    TASK_MANAGER_RESULT_QUEUE_TASK_NOT_FOUND,
    TASK_MANAGER_RESULT_OUT_OF_MEMORY,
    TASK_MANAGER_RESULT_CHILD_NOT_FOUND,
} task_manager_result_t;

typedef enum task_state_t {
    TASK_STATE_READY = 0,
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED,
    TASK_STATE_ZOMBIE,
    TASK_STATE_TERMINATED
} task_state_t;

typedef enum task_wait_reason_t {
    TASK_WAIT_REASON_NONE = 0,
    TASK_WAIT_REASON_CHILD,
    TASK_WAIT_REASON_IO
} task_wait_reason_t;

typedef struct task_t task_t;

typedef struct task_node_t {
    task_t* task;
    struct task_node_t* next;
    struct task_node_t* previous;
} task_node_t;

typedef struct task_t {
    uint32_t task_id;
    uint32_t task_esp;
    uint32_t task_cr3;

    task_state_t task_state;
    uint32_t task_time_slice;
    uint32_t task_queue_level;

    uint32_t stack_base;
    uint32_t stack_top;

    uint8_t task_is_user;

    struct task_t* next;
    
    uint8_t syscall_retry; // as i can't yield within the syscall code itself, the task retries the syscall after it gets unblocked

    // anything below i wont add into the asm file

    resource_t* resources[TASK_MAX_RESOURCES];
    uint32_t task_init_eip;
    struct task_t* parent;
    task_node_t* children;
    task_node_t* children_tail;
    int32_t return_code;
    uint8_t yield;
    syscall_execution_t syscall_execution;
    task_wait_reason_t wait_reason;
    void* wait_object;
} task_t;

typedef struct task_manager_t {
    task_t* task_current;

    task_t* task_next; // task to switch to
    uint32_t task_needs_switching;

    task_t* task_idle;
    task_t* task_queues[TASK_MANAGER_QUEUE_LEVELS + 2]; // +1 blocked, +2 zombies
    
    uint64_t last_priority_boost_at_ticks;
    task_t* task_init;
} task_manager_t;

extern task_manager_t task_manager;
void task_manager_module_init();

task_t* task_manager_task_create(task_manager_t* task_manager, void (*entry)(void), uint8_t task_is_user, uint8_t enqueue);
task_t* task_manager_task_copy(task_manager_t* task_manager, task_t* parent, uint8_t enqueue);
task_manager_result_t task_manager_prepare_new_stack(page_entry_t* task_page_directory, uint8_t task_is_user, uint32_t eip, uint32_t* out_esp);
task_manager_result_t task_manager_exit_task(task_manager_t* task_manager, task_t* task, int32_t return_code);
void task_manager_destroy_task(task_manager_t* task_manager, task_t* task);
task_manager_result_t task_manager_block_task(task_manager_t* task_manager, task_t* task, task_wait_reason_t wait_reason, void* wait_object);
task_manager_result_t task_manager_unblock_task(task_manager_t* task_manager, task_t* task);
void task_manager_wake_blocked_on(task_manager_t* task_manager, task_wait_reason_t wait_reason, void* wait_object);

task_manager_result_t task_manager_add_child_to_task(task_t* parent, task_t* child);
task_manager_result_t task_manager_remove_child_from_task(task_t* parent, task_t* child);

void task_manager_schedule(task_manager_t* task_manager);
task_t* task_manager_create_idle_task(task_manager_t* task_manager);
task_t* task_manager_pick_task(task_manager_t* task_manager);
task_t* task_manager_find_child(task_manager_t* task_manager, task_t* parent, uint32_t pid);
task_t* task_manager_find_zombie_child(task_manager_t* task_manager, task_t* parent);
uint32_t task_manager_calculate_time_slice(uint32_t queue_level);
void task_manager_boost_priority_of_all_tasks(task_manager_t* task_manager);
task_manager_result_t task_manager_yield_current(task_manager_t* task_manager);

void task_manager_requeue_task(task_manager_t* task_manager, task_t* task, uint32_t used_time_slice);
void task_manager_enqueue_task(task_manager_t* task_manager, size_t queue_index, task_t* task);
task_t* task_manager_dequeue_task(task_manager_t* task_manager, size_t queue_index);
task_manager_result_t task_manager_remove_task_from_queue(task_manager_t* task_manager, size_t queue_index, task_t* task);

#endif // ALURA_TASK_H