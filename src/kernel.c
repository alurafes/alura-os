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

static inline int syscall3(int n, int a1, int a2, int a3) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(n), "b"(a1), "c"(a2), "d"(a3)
        : "memory"
    );
    return ret;
}

kernel_result_t kernel_initialize(multiboot_info_t* multiboot)
{
    memory_bitmap_module_init(multiboot);
    memory_paging_module_init();
    kernel_heap_module_init();
    vga_driver_init();
    terminal_module_init(&vga.driver);
    gdt_module_init();
    tss_module_init();
    pic_module_init();
    idt_module_init();
    irq_module_init();
    timer_module_init();
    task_manager_module_init();

    multiboot_info_t* multiboot_virtual = (multiboot_info_t*)kernel_low_physical_to_virtual((void*)multiboot);
    framebuffer_driver_init(multiboot_virtual);
    ramfs_driver_init(multiboot_virtual);
    vfs_module_init();

    keyboard_driver_init();

    char* init_argv[] = { "/bin/init.elf", NULL };
    elf_load_and_execute("/bin/init.elf", init_argv, &task_manager.task_init);
    keyboard_open(task_manager.task_init, NULL); // stdin
    terminal_open(task_manager.task_init, NULL); // stdout
    terminal_open(task_manager.task_init, NULL); // stderr

    __asm__ volatile("sti");

    return KERNEL_RESULT_OK;
}

void kernel_main(multiboot_info_t* multiboot)
{
    kernel_initialize(multiboot);
}