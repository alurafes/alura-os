#ifndef ALURA_DRIVER_KEYBOARD_H
#define ALURA_DRIVER_KEYBOARD_H

#include "irq.h"

typedef struct keyboard_special_state_t {
    uint8_t shift;
    uint8_t ctrl;
    uint8_t alt;
} keyboard_special_state_t;

typedef struct keyboard_t {
    keyboard_special_state_t special_state;
} keyboard_t;

extern keyboard_t keyboard;
void keyboard_driver_init();

#endif // ALURA_DRIVER_KEYBOARD_H