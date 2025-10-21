#include "memory_paging.h"

#include "vga.h"

#define BOOTSTRAP_ALLOCATOR_PAGES 64 // quite wasteful but for now i don't care that much

static uint8_t __attribute__((section(".bootstrap.stack"), aligned(4096))) bootstrap_allocator_memory[BOOTSTRAP_ALLOCATOR_PAGES * PAGE_SIZE];
static uint32_t __attribute__((section(".bootstrap.stack"))) bootstrap_allocator_used_pages = 0;

static page_entry_t* __attribute__((section(".bootstrap.stack"))) bootstrap_page_directory = NULL;

__attribute__((section(".bootstrap.text")))
static void* bootstrap_allocator_allocate()
{
    if (bootstrap_allocator_used_pages > BOOTSTRAP_ALLOCATOR_PAGES) return NULL;
    void* page = &bootstrap_allocator_memory[bootstrap_allocator_used_pages * PAGE_SIZE];
    bootstrap_allocator_used_pages++;
    return page;
}

__attribute__((section(".bootstrap.text")))
static void reset_entry(page_entry_t* entry)
{
    for (uint32_t i = 0; i < PAGE_SIZE / sizeof(page_entry_t); ++i)
    {
        entry[i] = 0;
    }
}

__attribute__((section(".bootstrap.text")))
static void bootstrap_paging_map(uintptr_t physical_address, uintptr_t virtual_address, uint32_t flags)
{
    size_t page_directory_index = (virtual_address >> 22) & 0x3FF;
    size_t page_table_index = (virtual_address >> 12) & 0x3FF;
    
    uint32_t page_table_present = bootstrap_page_directory[page_directory_index] & PAGE_PRESENT;
    if (!page_table_present)
    {
        page_entry_t* new_page_table_entry = (page_entry_t*)bootstrap_allocator_allocate();
        if (new_page_table_entry == NULL) return;
        reset_entry(new_page_table_entry);
        bootstrap_page_directory[page_directory_index] = ((uint32_t)new_page_table_entry) | PAGE_PRESENT | PAGE_READ_WRITE;
    }
    page_entry_t* page_table = (page_entry_t*)(bootstrap_page_directory[page_directory_index] & 0xFFFFF000);
    page_table[page_table_index] = (physical_address) | (flags & 0xFFF) | PAGE_PRESENT;
}

__attribute__((section(".bootstrap.text")))
static void create_page_directory()
{
    bootstrap_page_directory = bootstrap_allocator_allocate();
    if (bootstrap_page_directory == NULL) return;
    reset_entry(bootstrap_page_directory);
}

__attribute__((section(".bootstrap.text")))
static void identity_map_first_megabyte()
{
    for (int i = 0; i < 0x100; ++i)
    {
        bootstrap_paging_map(i * PAGE_SIZE, i * PAGE_SIZE, PAGE_READ_WRITE);
    }
}

__attribute__((section(".bootstrap.text")))
static void identity_map_kernel()
{
    uintptr_t kernel_start = (uintptr_t)&_kernel_physical_start;
    uintptr_t kernel_end = (uintptr_t)&_kernel_physical_end;
    uintptr_t address = ALIGN_DOWN(kernel_start);
    for (; address < kernel_end; address += PAGE_SIZE)
    {
        bootstrap_paging_map(address, address, PAGE_READ_WRITE);
    }
}

__attribute__((section(".bootstrap.text")))
static void higher_half_map_kernel()
{
    uintptr_t kernel_start = (uintptr_t)&_kernel_physical_start;
    uintptr_t kernel_end = (uintptr_t)&_kernel_physical_end;
    uintptr_t address = ALIGN_DOWN(kernel_start);
    for (; address < kernel_end; address += PAGE_SIZE)
    {
        bootstrap_paging_map(address, KERNEL_VIRTUAL_START + address, PAGE_READ_WRITE);
    }
}

// giving it the theoretical max (32 pages for 128kb)
__attribute__((section(".bootstrap.text")))
static void higher_half_map_memory_bitmap()
{
    uintptr_t address = ALIGN_UP((uint32_t)&_kernel_physical_end);
    uintptr_t bitmap_end = address + 32 * PAGE_SIZE;
    for (; address < bitmap_end; address += PAGE_SIZE)
    {
        bootstrap_paging_map(address, KERNEL_VIRTUAL_START + address, PAGE_READ_WRITE);
    }
}

// VGA so far. Framebuffer in the future
__attribute__((section(".bootstrap.text")))
static void higher_half_map_extras()
{
    bootstrap_paging_map(0x00B0000, VGA_BUFFER, PAGE_READ_WRITE);
}

__attribute__((section(".bootstrap.text")))
static void enable_paging()
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(bootstrap_page_directory));
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0)); 
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

__attribute__((section(".bootstrap.text")))
void setup_bootstrap_paging()
{
    create_page_directory();
    identity_map_first_megabyte();
    identity_map_kernel();
    higher_half_map_kernel();
    higher_half_map_memory_bitmap();
    higher_half_map_extras();
    enable_paging();
}
