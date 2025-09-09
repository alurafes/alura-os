#include <stdint.h>
#include <stddef.h>

#include "kernel.h"

kernel_result_t kernel_initialize()
{
    vga_create(&kernel.vga);
    gdt_create(&kernel.gdt);
}

void kernel_main()
{
    kernel_initialize();

    vga_set_color(&kernel.vga, (vga_color_t){.background = VGA_COLOR_RED});
    vga_clear_color(&kernel.vga);

    vga_set_overflow(&kernel.vga, VGA_OVERFLOW_WRAP);
    vga_set_color(&kernel.vga, (vga_color_t){.background = VGA_COLOR_BLACK, .foreground = VGA_COLOR_PINK});
    vga_set_cursor(&kernel.vga, (vga_point_t){.x = 75, .y = 22});
    vga_put_string(&kernel.vga, "BABABOOEY");

    while (1) { __asm__ __volatile__("hlt"); }
}