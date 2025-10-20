#ifndef ALURA_VGA_H
#define ALURA_VGA_H

#include <stdint.h>
#include "display_driver.h"

#define VGA_BUFFER 0xC03FF000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_POINTER ((uint16_t*)VGA_BUFFER)

#define VGA_COLOR_BLACK 0x00
#define VGA_COLOR_BLUE 0x01
#define VGA_COLOR_GREEN 0x02
#define VGA_COLOR_CYAN 0x03
#define VGA_COLOR_RED 0x04
#define VGA_COLOR_MAGENTA 0x05
#define VGA_COLOR_BROWN 0x06
#define VGA_COLOR_LIGHT_GRAY 0x07
#define VGA_COLOR_DARK_GRAY 0x08
#define VGA_COLOR_LIGHT_BLUE 0x09
#define VGA_COLOR_LIGHT_GREEN 0x0A
#define VGA_COLOR_LIGHT_CYAN 0x0B
#define VGA_COLOR_LIGHT_RED 0x0C
#define VGA_COLOR_PINK 0x0D
#define VGA_COLOR_YELLOW 0x0E
#define VGA_COLOR_WHITE 0x0F

typedef struct vga_color_t {
    uint8_t background : 4;
    uint8_t foreground : 4;
} vga_color_t;

#define VGA_COLOR(color) (color.background << 4 | color.foreground)

typedef enum vga_result_t {
    VGA_RESULT_OK,
} vga_result_t;

typedef struct vga_t {
    display_driver_t driver;
    vga_color_t color;
} vga_t;

vga_result_t vga_create(vga_t* out);
vga_result_t vga_set_color(vga_t* vga, vga_color_t color);
void vga_put_char(display_driver_t* driver, char character, unsigned int x, unsigned int y);

extern vga_t vga;
void vga_module_init();

#endif // ALURA_VGA_H