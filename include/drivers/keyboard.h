#ifndef ALURA_DRIVER_KEYBOARD_H
#define ALURA_DRIVER_KEYBOARD_H

#include "irq.h"
#include "io.h"
#include "resourse.h"

#define KEYBOARD_BUFFER_SIZE 256

typedef struct keyboard_special_state_t {
    uint8_t shift;
    uint8_t ctrl;
    uint8_t alt;
} keyboard_special_state_t;

typedef struct keyboard_t {
    keyboard_special_state_t special_state;
    uint8_t buffer[KEYBOARD_BUFFER_SIZE];
    size_t buffer_head;
    size_t buffer_tail;
    size_t buffer_count;
    
} keyboard_t;

extern keyboard_t keyboard;
void keyboard_driver_init();

resource_result_t keyboard_open(task_t* task, size_t* result);

#endif // ALURA_DRIVER_KEYBOARD_H