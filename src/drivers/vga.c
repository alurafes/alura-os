#include "drivers/vga.h"

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
    out->driver.set_cursor = vga_set_cursor;
    return VGA_RESULT_OK;
}

void vga_cursor_disable()
{
    io_outb(0x3D4, 0x0A);
    io_outb(0x3D5, 0x20);
}

void vga_set_cursor(display_driver_t* driver, unsigned int x, unsigned int y)
{
    vga_t* vga = (vga_t*)driver;
    
    uint16_t pos = y * VGA_WIDTH + x;
	io_outb(0x3D4, 0x0F);
	io_outb(0x3D5, (uint8_t) (pos & 0xFF));
	io_outb(0x3D4, 0x0E);
	io_outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

vga_t vga;
void vga_driver_init()
{
    vga_create(&vga);
}