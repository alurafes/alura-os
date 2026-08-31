#ifndef ALURA_DRIVER_FRAMEBUFFER_H
#define ALURA_DRIVER_FRAMEBUFFER_H

#include "multiboot.h"

#include <stdint.h>
#include "text_display_driver.h"
#include "io.h"

typedef enum framebuffer_result_t {
    FRAMEBUFFER_RESULT_OK = 0
} framebuffer_result_t;

typedef struct framebuffer_color_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} framebuffer_color_t;

typedef struct framebuffer_t {
    text_display_driver_t driver;
    uint32_t address;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint32_t size;
    uint8_t red_field_position;
    uint8_t green_field_position;
    uint8_t blue_field_position;
    framebuffer_color_t foreground;
    framebuffer_color_t background;
    unsigned int cursor_x;
    unsigned int cursor_y;
} framebuffer_t;

typedef struct framebuffer_sprite_t {
    uint32_t* pixels; // 32 bit pixels 0xRRGGBB
    uint32_t width;
    uint32_t height;
} framebuffer_sprite_t;

framebuffer_result_t framebuffer_create(framebuffer_t* out);
framebuffer_result_t framebuffer_set_color(framebuffer_t* framebuffer, framebuffer_color_t foreground, framebuffer_color_t background);
void framebuffer_put_char(text_display_driver_t* driver, char character, unsigned int x, unsigned int y);
void framebuffer_set_cursor(text_display_driver_t* driver, unsigned int x, unsigned int y);
void framebuffer_get_dimensions(text_display_driver_t* driver, unsigned int* width, unsigned int* height);
void framebuffer_put_pixel(framebuffer_t* framebuffer, unsigned int x, unsigned int y, uint8_t r, uint8_t g, uint8_t b);
void framebuffer_blit(framebuffer_t* framebuffer, int dest_x, int dest_y, framebuffer_sprite_t* source);

extern framebuffer_t framebuffer;
void framebuffer_driver_init(multiboot_info_t* multiboot);

#endif // ALURA_DRIVER_FRAMEBUFFER_H