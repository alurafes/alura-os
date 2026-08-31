#ifndef ALURA_TERMINAL_H
#define ALURA_TERMINAL_H

#include <stdint.h>
#include "text_display_driver.h"
#include "resourse.h"
#include "kernel_heap.h"

typedef struct terminal_point_t {
    int x;
    int y;
} terminal_point_t;

typedef enum terminal_overflow_t {
    TERMINAL_OVERFLOW_NONE = 0,
    TERMINAL_OVERFLOW_WRAP,
    TERMINAL_OVERFLOW_NEW_LINE,
} terminal_overflow_t;

typedef enum terminal_scroll_t {
    TERMINAL_SCROLL_NONE = 0,
    TERMINAL_SCROLL_VERTICAL,
} terminal_scroll_t;

typedef struct terminal_t {
    text_display_driver_t* driver;
    char* buffer;
    terminal_point_t cursor;
    terminal_overflow_t overflow;
    terminal_scroll_t scroll;
    unsigned int width;
    unsigned int height;
} terminal_t;

typedef enum terminal_result_t {
    TERMINAL_RESULT_OK = 0,
    TERMINAL_RESULT_OUT_OF_BOUNDS,
} terminal_result_t;

terminal_result_t terminal_create(terminal_t* out, text_display_driver_t* driver);
terminal_result_t terminal_set_cursor(terminal_t* terminal, terminal_point_t point);
terminal_result_t terminal_set_overflow(terminal_t* terminal, terminal_overflow_t overflow);
terminal_result_t terminal_set_scroll(terminal_t* terminal, terminal_scroll_t scroll);
terminal_result_t terminal_scroll(terminal_t* terminal);
terminal_result_t terminal_put_char(terminal_t* terminal, char character);
terminal_result_t terminal_put_string(terminal_t* terminal, const char* string);
terminal_result_t terminal_render(terminal_t* terminal);

extern terminal_t terminal;
void terminal_module_init(text_display_driver_t* driver);

resource_result_t terminal_open(task_t* task, size_t* result);

#endif // ALURA_TERMINAL_H