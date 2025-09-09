#include <stdint.h>
#include <stddef.h>

#include "kernel.h"

kernel_result_t kernel_initialize()
{
    vga_create(&kernel.vga);
    terminal_create(&kernel.terminal, &kernel.vga.driver);
    gdt_create(&kernel.gdt);
}

void kernel_main()
{
    kernel_initialize();

    vga_set_color(&kernel.vga, (vga_color_t){.background = VGA_COLOR_GREEN, .foreground = VGA_COLOR_BLACK});
    terminal_render(&kernel.terminal);
    terminal_put_string(&kernel.terminal, "aaaaThis is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! This is a test! vvvvv");
 
    while (1) { __asm__ __volatile__("hlt"); }
}