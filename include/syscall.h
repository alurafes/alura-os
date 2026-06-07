#ifndef ALURA_SYSCALL_H
#define ALURA_SYSCALL_H

#include "task_manager.h"

#include "idt.h"

// todo: figure out a better way

#define SYSCALL_OPEN 0
#define SYSCALL_CLOSE 1
#define SYSCALL_READ 2

void syscall_handler(register_interrupt_data_t* data);

#endif // ALURA_SYSCALL_H