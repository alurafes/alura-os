#include "kernel.h"

__attribute__((noreturn))
void simple_task()
{
    int x = 0;
    for (;;) {
        x++;
        uint32_t cs = 0;
        __asm__ volatile("mov %%cs, %0" : "=r"(cs)); 
        printf("cs: %x\n", cs);
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
    tss_module_init();
    pic_module_init();
    idt_module_init();
    irq_module_init();
    timer_module_init();
    task_manager_module_init();

    task_manager_task_create(&task_manager, simple_task, 0);
    task_manager_task_create(&task_manager, simple_task, 1);
    task_manager_task_create(&task_manager, simple_task, 1);


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