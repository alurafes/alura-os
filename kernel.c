#include "kernel.h"

kernel_result_t kernel_initialize(multiboot_info_t* multiboot)
{
    vga_module_init();
    terminal_module_init(&vga.driver);
    gdt_module_init();
    pic_init_module();
    idt_module_init();
    memory_bitmap_module_init(multiboot);
    __asm__ volatile("sti");
}

void kernel_main(multiboot_info_t* multiboot, uint32_t magic)
{
    kernel_initialize(multiboot);

    void* a = memory_bitmap_allocate();
    void* b = memory_bitmap_allocate();

    memory_bitmap_free(b);

    void* c = memory_bitmap_allocate();

    printf("a: %x, b: %x, c: %x\n", a, b, c);
    
    while (1) {  
        __asm__ volatile("hlt"); 
    }
}