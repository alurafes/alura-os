#ifndef ALURA_SYSCALL_H
#define ALURA_SYSCALL_H

#include "idt.h"
#include "memory.h"

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
#define SYSCALL_SBRK 8
#define SYSCALL_ISATTY 9
#define SYSCALL_GETPID 10
#define SYSCALL_LSEEK 11

#define SYSCALL_LSEEK_SET 0
#define SYSCALL_LSEEK_CUR 1
#define SYSCALL_LSEEK_END 2

#define SYSCALL_O_RDONLY 0x0000
#define SYSCALL_O_WRONLY 0x0001
#define SYSCALL_O_RDWR   0x0002
#define SYSCALL_O_APPEND 0x0008
#define SYSCALL_O_CREAT  0x0200
#define SYSCALL_O_TRUNC  0x0400
#define SYSCALL_O_EXCL   0x0800

#define SYSCALL_TASK (syscall.caller_task)
#define SYSCALL_GET_PARAMETER(index) (syscall.caller_task->syscall_execution.parameters[index])

typedef struct task_t task_t;

typedef enum syscall_result_t {
    SYSCALL_RESULT_OK = 0,
    SYSCALL_RESULT_FAIL,
    SYSCALL_RESULT_BAD_PARAMETER,
    SYSCALL_RESULT_BUSY,
    SYSCALL_RESULT_OUT_OF_MEMORY,
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