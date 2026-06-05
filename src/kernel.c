#include "kernel.h"

static inline int syscall1(int n, int a1) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(n), "b"(a1)
        : "memory"
    );
    return ret;
}

void simple_task()
{
    const char* path = "/dir_a/test_file";
    int result = syscall1(SYSCALL_OPEN, (int)path);
    printf("result: %d\n", result);
    result = syscall1(SYSCALL_CLOSE, result);
    printf("result: %d\n", result);
    while (1) {  
        __asm__ volatile("hlt"); 
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
    ramfs_driver_init();
    vfs_module_init();

    printf("alura-os is loaded!\n");
    __asm__ volatile("sti");

    task_manager_task_create(&task_manager, simple_task, 0);

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