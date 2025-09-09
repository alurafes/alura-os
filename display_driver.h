#ifndef ALURA_DISPLAY_DRIVER_H
#define ALURA_DISPLAY_DRIVER_H


typedef struct display_driver_t display_driver_t;

struct display_driver_t {
    void (*put_char)(display_driver_t* driver, char character, unsigned int x, unsigned int y);
};

#endif // ALURA_DISPLAY_DRIVER_H