#ifndef ALURA_TEXT_DISPLAY_DRIVER_H
#define ALURA_TEXT_DISPLAY_DRIVER_H


typedef struct text_display_driver_t text_display_driver_t;

struct text_display_driver_t {
    void (*put_char)(text_display_driver_t* driver, char character, unsigned int x, unsigned int y);
    void (*set_cursor)(text_display_driver_t* driver, unsigned int x, unsigned int y);
    void (*get_dimensions)(text_display_driver_t* driver, unsigned int* width, unsigned int* height);
};

#endif // ALURA_TEXT_DISPLAY_DRIVER_H