#include "kernel.h"

kernel_result_t kernel_initialize(multiboot_info_t* multiboot)
{
    memory_bitmap_module_init(multiboot);
    memory_paging_module_init();
    kernel_heap_module_init();
    vga_module_init();
    terminal_module_init(&vga.driver);
    gdt_module_init();
    tss_module_init();
    pic_module_init();
    idt_module_init();
    irq_module_init();
    timer_module_init();
    task_manager_module_init();
    vfs_module_init();

    __asm__ volatile("int $0x80"); 

    printf("alura-os is loaded!\n");

    __asm__ volatile("sti");
    return KERNEL_RESULT_OK;
}

void kernel_halt()
{
    while (1) {  
        __asm__ volatile("hlt"); 
    }
}

void kernel_main(multiboot_info_t* multiboot)
{
    kernel_initialize(multiboot);
    kernel_halt();
}