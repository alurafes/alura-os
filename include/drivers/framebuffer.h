#ifndef ALURA_DRIVER_FRAMEBUFFER_H
#define ALURA_DRIVER_FRAMEBUFFER_H

#include "multiboot.h"

#include <stdint.h>
#include "display_driver.h"
#include "io.h"

typedef enum framebuffer_result_t {
    FRAMEBUFFER_RESULT_OK = 0
} framebuffer_result_t;

typedef struct framebuffer_t {
    display_driver_t driver;
    uint32_t address;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint32_t size;
} framebuffer_t;

framebuffer_result_t framebuffer_create(framebuffer_t* out);
void framebuffer_put_char(display_driver_t* driver, char character, unsigned int x, unsigned int y);
void framebuffer_set_cursor(display_driver_t* driver, unsigned int x, unsigned int y);

extern framebuffer_t framebuffer;
void framebuffer_driver_init(multiboot_info_t* multiboot);

#endif // ALURA_DRIVER_FRAMEBUFFER_H