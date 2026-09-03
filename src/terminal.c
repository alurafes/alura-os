#include "terminal.h"

static char terminal_display_char(char character)
{
    switch (character) {
        case '\n':
            return ' ';
    }
    return character;
}

terminal_result_t terminal_set_cursor(terminal_t* terminal, terminal_point_t point)
{
    if (point.x < 0 || point.x >= terminal->width ||
        point.y < 0 || point.y >= terminal->height)
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
                if (point.y + 1 >= terminal->height)
                {
                    terminal_scroll(terminal);
                    point.y = terminal->height - 1;
                } else {
                    point.y += 1;
                }
                break;
            }
        }
    char previous_character = terminal->buffer[terminal->cursor.y * terminal->width + terminal->cursor.x];
    terminal->driver->put_char(terminal->driver, terminal_display_char(previous_character), terminal->cursor.x, terminal->cursor.y);
    terminal->cursor = point;
    terminal->driver->set_cursor(terminal->driver, terminal->cursor.x, terminal->cursor.y);
    return TERMINAL_RESULT_OK;
}

terminal_result_t terminal_put_char(terminal_t* terminal, char character)
{
    switch (character) {
        case '\n': {
            terminal->buffer[terminal->cursor.y * terminal->width + terminal->cursor.x] = character;
            terminal_set_cursor(terminal, (terminal_point_t){0, terminal->cursor.y + 1});
            return TERMINAL_RESULT_OK;
        }
        case '\b': {
            if (terminal->cursor.x == 0) return TERMINAL_RESULT_OK;
            terminal_point_t previous_point = {terminal->cursor.x - 1, terminal->cursor.y};
            terminal->buffer[previous_point.y * terminal->width + previous_point.x] = ' ';
            terminal->driver->put_char(terminal->driver, ' ', previous_point.x, previous_point.y);
            terminal_set_cursor(terminal, previous_point);
            return TERMINAL_RESULT_OK;
        }
    }
    terminal->buffer[terminal->cursor.y * terminal->width + terminal->cursor.x] = character;
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
    for (int y = 1; y < terminal->height; ++y)
    {
        for (int x = 0; x < terminal->width; ++x)
        {
            terminal->buffer[(y - 1) * terminal->width + x] = terminal->buffer[y * terminal->width + x];
        }
    }
    for (int x = 0; x < terminal->width; ++x)
    {
        terminal->buffer[(terminal->height - 1) * terminal->width + x] = 0x0;
    }
    return terminal_render(terminal);
}

terminal_result_t terminal_render(terminal_t* terminal)
{
    for (int y = 0; y < terminal->height; ++y)
    {
        for (int x = 0; x < terminal->width; ++x)
        {
            char character = terminal_display_char(terminal->buffer[y * terminal->width + x]);
            terminal->driver->put_char(terminal->driver, character, x, y);
        }
    }
    return TERMINAL_RESULT_OK;
}

terminal_result_t terminal_create(terminal_t* out, text_display_driver_t* driver)
{
    terminal_t t = {
        .buffer = NULL,
        .cursor = {
            .x = 0,
            .y = 0
        },
        .driver = driver,
        .overflow = TERMINAL_OVERFLOW_NEW_LINE,
        .scroll = TERMINAL_SCROLL_VERTICAL
    };
    driver->get_dimensions(driver, &t.width, &t.height);
    t.buffer = kernel_heap_calloc(sizeof(char) * t.width * t.height);
    terminal_render(&t);
    *out = t;
    return TERMINAL_RESULT_OK;
}

terminal_t terminal;
void terminal_module_init(text_display_driver_t* driver)
{
    terminal_create(&terminal, driver);
}

resource_result_t terminal_write(resource_t* resource, size_t offset, void* buffer, size_t length, size_t* written_bytes)
{
    (void)offset; // terminal is a stream, not seekable
    terminal_t* terminal = (terminal_t*)resource->data;
    const char* data = (const char*)buffer;

    size_t written = 0;
    while (written < length && data[written])
    {
        terminal_put_char(terminal, data[written]);
        written++;
    }

    if (written_bytes != NULL) *written_bytes = written;

    return RESOURCE_RESULT_OK;
}

resource_operations_t terminal_operations = {
    .write = terminal_write
};

resource_result_t terminal_open(task_t* task, size_t* result)
{
    return resource_register(task, RESOURCE_TYPE_TERMINAL, &terminal, &terminal_operations, result);
}