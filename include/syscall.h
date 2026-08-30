#ifndef ALURA_SYSCALL_H
#define ALURA_SYSCALL_H

#include "idt.h"

// todo: figure out a better way

#define SYSCALL_MAX_PARAMETERS 6

#define SYSCALL_OPEN 0
#define SYSCALL_CLOSE 1
#define SYSCALL_READ 2
#define SYSCALL_WRITE 3
#define SYSCALL_FORK 4
#define SYSCALL_EXECVE 5
#define SYSCALL_EXIT 6
#define SYSCALL_WAITPID 7

#define SYSCALL_TASK (syscall.caller_task)
#define SYSCALL_GET_PARAMETER(index) (syscall.caller_task->syscall_execution.parameters[index])

typedef struct task_t task_t;

typedef enum syscall_result_t {
    SYSCALL_RESULT_OK = 0,
    SYSCALL_RESULT_FAIL,
    SYSCALL_RESULT_BAD_PARAMETER,
    SYSCALL_RESULT_BUSY,
} syscall_result_t;

typedef struct syscall_execution_t {
    uint32_t index;
    intptr_t parameters[SYSCALL_MAX_PARAMETERS];
} syscall_execution_t;

typedef struct syscall_execution_state_write_t {
    uint32_t offset;
} syscall_execution_state_write_t;

typedef struct syscall_execution_state_t {
    syscall_execution_state_write_t write;
} syscall_execution_state_t;

typedef struct syscall_t {
    register_interrupt_data_t* current_register_data;
    task_t* caller_task;
} syscall_t;

extern syscall_t syscall;

void syscall_handler(register_interrupt_data_t* data);

#endif // ALURA_SYSCALL_H