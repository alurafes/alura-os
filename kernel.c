#include "kernel.h"

kernel_result_t kernel_initialize()
{
    gdt_module_init();
    vga_module_init();
    terminal_module_init(&vga.driver);
}

void kernel_main()
{
    kernel_initialize();

    printf("GDT at: %x\nVGA at: %x\nTerminal at: %x", &gdt, &vga, &terminal);

    while (1) {  
        __asm__ __volatile__("hlt"); 
    }
}