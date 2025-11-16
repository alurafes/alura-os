#include "kernel.h"

void task_a()
{
    while (1) {
        printf("Task A: %x\n", task_manager.task_current);
    }
}

void task_b()
{
    while (1) {  
        printf("Task B: %x\n", task_manager.task_current);
    }
}

kernel_result_t kernel_initialize(multiboot_info_t* multiboot)
{
    memory_bitmap_module_init(multiboot);
    memory_paging_module_init();
    kernel_heap_module_init();
    vga_module_init();
    terminal_module_init(&vga.driver);
    gdt_module_init();
    pic_module_init();
    idt_module_init();
    irq_module_init();
    timer_module_init();
    task_manager_module_init();

    task_manager_task_create(&task_manager, task_a);
    task_manager_task_create(&task_manager, task_b);

    printf("alura-os is loaded!\n");
    __asm__ volatile("sti");

    return KERNEL_RESULT_OK;
}

void kernel_main(multiboot_info_t* multiboot)
{
    kernel_initialize(multiboot);

    while (1) {  
        __asm__ volatile("hlt"); 
    }
}