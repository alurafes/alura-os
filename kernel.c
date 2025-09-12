#include "kernel.h"

kernel_result_t kernel_initialize()
{
    gdt_module_init();
    idt_module_init();
    vga_module_init();
    terminal_module_init(&vga.driver);
}

void kernel_main()
{
    kernel_initialize();

    vga_set_color(&vga, (vga_color_t){.background = VGA_COLOR_DARK_GRAY, .foreground = VGA_COLOR_CYAN});

    terminal_render(&terminal);

    printf("GDT at: %x\nIDT at: %x\nVGA at: %x\nTerminal at: %x", &gdt, &idt, &vga, &terminal);

    while (1) {  
        __asm__ __volatile__("hlt"); 
    }
}