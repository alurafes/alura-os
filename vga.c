#include "vga.h"

#include "stddef.h"

vga_result_t vga_set_color(vga_t* vga, vga_color_t color)
{
    vga->color = color;
    return VGA_RESULT_OK;
}

void vga_put_char(display_driver_t* driver, char character, unsigned int x, unsigned int y)
{
    vga_t* vga = (vga_t*)driver;
    VGA_POINTER[VGA_WIDTH * y + x] = VGA_COLOR(vga->color) << 8 | character;
}

vga_result_t vga_create(vga_t* out)
{
    out->color.background = VGA_COLOR_BLACK;
    out->color.foreground = VGA_COLOR_WHITE;
    out->driver.put_char = vga_put_char;
    return VGA_RESULT_OK;
}