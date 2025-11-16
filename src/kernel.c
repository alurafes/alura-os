#include "kernel.h"

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

    __asm__ volatile("sti");

    printf("alura-os is loaded!\n");

    return KERNEL_RESULT_OK;
}

void task_b()
{
    printf("Task B\n");
    while (1) {  
        __asm__ volatile("hlt"); 
    }
}

void task_a()
{
    printf("Task A\n");

    task_t* task = task_manager_task_create(task_b);
    printf("stack %x\nfunc %x\n", task->esp, task_a);
    task_manager_task_switch(task);

    while (1) {  
        __asm__ volatile("hlt"); 
    }
}

void kernel_main(multiboot_info_t* multiboot)
{
    kernel_initialize(multiboot);

    task_t* task = task_manager_task_create(task_a);
    printf("stack %x\nfunc %x\n", task->esp, task_a);
    task_manager_task_switch(task);

    while (1) {  
        __asm__ volatile("hlt"); 
    }
}