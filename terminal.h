#ifndef ALURA_TERMINAL_H
#define ALURA_TERMINAL_H

#include <stdint.h>
#include "display_driver.h"

#define TERMINAL_WIDTH 80
#define TERMINAL_HEIGHT 25

typedef struct terminal_point_t {
    int x;
    int y;
} terminal_point_t;

typedef enum terminal_overflow_t {
    TERMINAL_OVERFLOW_NONE,
    TERMINAL_OVERFLOW_WRAP,
    TERMINAL_OVERFLOW_NEW_LINE,
} terminal_overflow_t;

typedef enum terminal_scroll_t {
    TERMINAL_SCROLL_NONE,
    TERMINAL_SCROLL_VERTICAL,
} terminal_scroll_t;

typedef struct terminal_t {
    display_driver_t* driver;
    char buffer[TERMINAL_WIDTH * TERMINAL_HEIGHT];
    terminal_point_t cursor;
    terminal_overflow_t overflow;
    terminal_scroll_t scroll;
} terminal_t;

typedef enum terminal_result_t {
    TERMINAL_RESULT_OK,
    TERMINAL_RESULT_OUT_OF_BOUNDS,
} terminal_result_t;

terminal_result_t terminal_create(terminal_t* out, display_driver_t* driver);
terminal_result_t terminal_set_cursor(terminal_t* terminal, terminal_point_t point);
terminal_result_t terminal_set_overflow(terminal_t* terminal, terminal_overflow_t overflow);
terminal_result_t terminal_set_scroll(terminal_t* terminal, terminal_scroll_t scroll);
terminal_result_t terminal_scroll(terminal_t* terminal);
terminal_result_t terminal_put_char(terminal_t* terminal, char character);
terminal_result_t terminal_put_string(terminal_t* terminal, const char* string);
terminal_result_t terminal_render(terminal_t* terminal);

#endif // ALURA_TERMINAL_H