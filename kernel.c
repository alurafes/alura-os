#include "kernel.h"

kernel_result_t kernel_initialize()
{
    vga_module_init();
    terminal_module_init(&vga.driver);
    gdt_module_init();
    pic_init_module();
    idt_module_init();
    __asm__ volatile("sti");
}

extern char _kernel_start;
extern char _kernel_end;

#define ALIGN_TO_PAGE(value, page_size) (value + page_size - 1) & ~(page_size - 1)

#define PAGE_SIZE 4096

typedef uint32_t bitmap_entry_t;

static bitmap_entry_t* memory_bitmap;
static size_t memory_bitmap_pages;

#define BITMAP_ENTRY_SIZE_IN_BITS (sizeof(bitmap_entry_t) * 8)
#define BITMAP_SET(bitmap, page_index) (bitmap[page_index / BITMAP_ENTRY_SIZE_IN_BITS] |=  1 << (page_index % BITMAP_ENTRY_SIZE_IN_BITS))
#define BITMAP_CLEAR(bitmap, page_index) (bitmap[page_index / BITMAP_ENTRY_SIZE_IN_BITS] &= ~(1 << (page_index % BITMAP_ENTRY_SIZE_IN_BITS)))
#define BITMAP_TEST(bitmap, page_index) (bitmap[page_index / BITMAP_ENTRY_SIZE_IN_BITS] & 1 << (page_index % BITMAP_ENTRY_SIZE_IN_BITS))

void kernel_main(multiboot_info_t* multiboot, uint32_t magic)
{
    kernel_initialize();

    size_t total_memory_bytes = 0;
    for (int i = 0; i < multiboot->mmap_length; i += sizeof(multiboot_memory_map_t)) 
    {
        multiboot_memory_map_t* memory_map = (multiboot_memory_map_t*)(multiboot->mmap_addr + i); 
        size_t region_end = memory_map->len + memory_map->addr;
        if (region_end > total_memory_bytes) total_memory_bytes = region_end;   
    }

    memory_bitmap_pages = total_memory_bytes / PAGE_SIZE;
    memory_bitmap = (uint32_t*)(ALIGN_TO_PAGE((uint32_t)&_kernel_end, PAGE_SIZE));

    for (int page_index = 0; page_index < memory_bitmap_pages; ++page_index)
    {
        BITMAP_SET(memory_bitmap, page_index);
    }

    for (int i = 0; i < multiboot->mmap_length; i += sizeof(multiboot_memory_map_t)) 
    {
        multiboot_memory_map_t* memory_map = (multiboot_memory_map_t*)(multiboot->mmap_addr + i); 
        if (memory_map->type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            size_t page = memory_map->addr / PAGE_SIZE;
            size_t last_page = (memory_map->addr + memory_map->len + PAGE_SIZE - 1) / PAGE_SIZE;
            for (; page < last_page; ++page)
            {
                BITMAP_CLEAR(memory_bitmap, page);
            }
        }
    }

    size_t page = 0;
    size_t last_page = 0;
    // lock first megabyte
    last_page = 0x100000 / PAGE_SIZE;
    for (size_t page = 0; page < last_page; ++page)
    {
        BITMAP_SET(memory_bitmap, page);
    }

    // lock kernel
    page = (uint32_t)&_kernel_start / PAGE_SIZE;
    last_page = ((uint32_t)&_kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
    for (; page < last_page; ++page)
    {
        BITMAP_SET(memory_bitmap, page);
    }

    // lock memory bitmap
    size_t bitmap_start_page = (uint32_t)memory_bitmap / PAGE_SIZE;
    for (page = 0; page < memory_bitmap_pages; ++page)
    {
        BITMAP_SET(memory_bitmap, bitmap_start_page + page);
    }

    printf("bitmap at %x = %d\n", 0x115000, BITMAP_TEST(memory_bitmap, 0x115000 / PAGE_SIZE) > 0);

    while (1) {  
        __asm__ volatile("hlt"); 
    }
}