#include "terminal.h"


terminal_result_t terminal_set_cursor(terminal_t* terminal, terminal_point_t point)
{
    if (point.x < 0 || point.x >= TERMINAL_WIDTH ||
        point.y < 0 || point.y >= TERMINAL_HEIGHT) 
        {
            switch (terminal->overflow)
            {
            case TERMINAL_OVERFLOW_NONE:
                return TERMINAL_RESULT_OUT_OF_BOUNDS;
            case TERMINAL_OVERFLOW_WRAP:
                point.x = 0;
                break;
            case TERMINAL_OVERFLOW_NEW_LINE:
                point.x = 0;
                if (point.y + 1 >= TERMINAL_HEIGHT)
                {
                    terminal_scroll(terminal);
                    point.y = TERMINAL_HEIGHT - 1;
                } else {
                    point.y += 1;
                }
                break;
            }
        }
    terminal->cursor = point;
    return TERMINAL_RESULT_OK;
}

terminal_result_t terminal_put_char(terminal_t* terminal, char character)
{
    terminal->buffer[terminal->cursor.y * TERMINAL_WIDTH + terminal->cursor.x] = character;
    switch (character) {
        case '\n': {
            terminal_set_cursor(terminal, (terminal_point_t){0, terminal->cursor.y + 1});
            return TERMINAL_RESULT_OK;
        }
    }
    terminal->driver->put_char(terminal->driver, character, terminal->cursor.x, terminal->cursor.y);
    terminal_point_t next_point = {
        .x = terminal->cursor.x + 1,
        .y = terminal->cursor.y
    };
    terminal_result_t result = terminal_set_cursor(terminal, next_point);
    if (result != TERMINAL_RESULT_OK) return result;
    return TERMINAL_RESULT_OK;
}

terminal_result_t terminal_put_string(terminal_t* terminal, const char* string)
{
    for (int i = 0; string[i] != '\0'; ++i)
    {
        terminal_result_t result = terminal_put_char(terminal, string[i]);
        if (result != TERMINAL_RESULT_OK) return result;
    }
    return TERMINAL_RESULT_OK;
}

terminal_result_t terminal_set_overflow(terminal_t* terminal, terminal_overflow_t overflow)
{
    terminal->overflow = overflow;
    return TERMINAL_RESULT_OK;
}
terminal_result_t terminal_set_scroll(terminal_t* terminal, terminal_scroll_t scroll)
{
    terminal->scroll = scroll;
    return TERMINAL_RESULT_OK;
}

terminal_result_t terminal_scroll(terminal_t* terminal)
{
    for (int y = 1; y < TERMINAL_HEIGHT; ++y)
    {
        for (int x = 0; x < TERMINAL_WIDTH; ++x)
        {
            terminal->buffer[(y - 1) * TERMINAL_WIDTH + x] = terminal->buffer[y * TERMINAL_WIDTH + x];
        }
    }
    for (int x = 0; x < TERMINAL_WIDTH; ++x)
    {
        terminal->buffer[(TERMINAL_HEIGHT - 1) * TERMINAL_WIDTH + x] = 0x0;
    }
    return terminal_render(terminal);
}

terminal_result_t terminal_render(terminal_t* terminal)
{
    for (int y = 0; y < TERMINAL_HEIGHT; ++y)
    {
        for (int x = 0; x < TERMINAL_WIDTH; ++x)
        {
            char character = terminal->buffer[y * TERMINAL_WIDTH + x];
            switch (character) {
                case '\n': {
                    character = 0x20;
                }
            }
            terminal->driver->put_char(terminal->driver, character, x, y);
        }
    }
    return TERMINAL_RESULT_OK;
}

terminal_result_t terminal_create(terminal_t* out, display_driver_t* driver)
{
    terminal_t t = {
        .buffer = {0},
        .cursor = {
            .x = 0,
            .y = 0
        },
        .driver = driver,
        .overflow = TERMINAL_OVERFLOW_NEW_LINE,
        .scroll = TERMINAL_SCROLL_VERTICAL
    };
    terminal_render(&t);
    *out = t;
    return TERMINAL_RESULT_OK;
}

terminal_t terminal;
void terminal_module_init(display_driver_t* driver)
{
    terminal_create(&terminal, driver);
}