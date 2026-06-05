#ifndef ALURA_SYSCALL_H
#define ALURA_SYSCALL_H

typedef struct register_interrupt_data_t register_interrupt_data_t;

#include "task_manager.h"

// todo: figure out a better way

#define SYSCALL_OPEN 0
#define SYSCALL_CLOSE 1

void syscall_handler(register_interrupt_data_t* data);

#endif // ALURA_SYSCALL_H