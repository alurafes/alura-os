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

    vga_set_color(&kernel.vga, (vga_color_t){.background = VGA_COLOR_GREEN, .foreground = VGA_COLOR_BLACK});
    vga_clear_color(&kernel.vga);

    vga_set_overflow(&kernel.vga, VGA_OVERFLOW_NEW_LINE);
    vga_set_scroll(&kernel.vga, VGA_SCROLL_VERTICAL);
    vga_set_cursor(&kernel.vga, (vga_point_t){.x = 0, .y = 0});
    vga_put_string(&kernel.vga, "This is a test!");
 
    while (1) { __asm__ __volatile__("hlt"); }
}