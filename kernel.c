#include "kernel.h"

kernel_result_t kernel_initialize(multiboot_info_t* multiboot)
{
    memory_bitmap_module_init(multiboot);
    memory_paging_module_init(&memory_bitmap);
    kernel_heap_module_init(KERNEL_HEAP_VIRTUAL_START, KERNEL_HEAP_VIRTUAL_END);
    vga_module_init();
    terminal_module_init(&vga.driver);
    gdt_module_init();
    pic_module_init();
    idt_module_init();
    __asm__ volatile("sti");
    printf("alura-os is loaded!\n");
}

void kernel_main(multiboot_info_t* multiboot, uint32_t magic)
{
    kernel_initialize(multiboot);

    while (1) {  
        __asm__ volatile("hlt"); 
    }
}