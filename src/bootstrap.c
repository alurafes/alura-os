#include "memory_paging.h"

#include "vga.h"

#define BOOTSTRAP_4MB_PAGES 16

static page_entry_t __attribute__((section(".bootstrap.data"), aligned(4096))) bootstrap_page_directory[1024];

__attribute__((section(".bootstrap.text")))
static inline void reset_entry(page_entry_t* entry)
{
    for (uint32_t i = 0; i < PAGE_SIZE / sizeof(page_entry_t); ++i)
    {
        entry[i] = 0;
    }
}

__attribute__((section(".bootstrap.text")))
static inline void enable_pse() {
    uint32_t cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= 0x00000010;
    __asm__ volatile("mov %0, %%cr4" :: "r"(cr4));
}

__attribute__((section(".bootstrap.text")))
static inline void enable_paging()
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(bootstrap_page_directory));
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0)); 
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

__attribute__((section(".bootstrap.text")))
static inline void map_pages()
{
    for (uint32_t i = 0; i < BOOTSTRAP_4MB_PAGES; i++) {
        uint32_t phys_addr = i * 0x400000;
        uint32_t entry = phys_addr | PAGE_PRESENT | PAGE_READ_WRITE | PAGE_4MB;
        
        // Identity map
        bootstrap_page_directory[i] = entry;
        // Higher half
        bootstrap_page_directory[768 + i] = entry;
    }
}


__attribute__((section(".bootstrap.text")))
void setup_bootstrap_paging()
{
    reset_entry(bootstrap_page_directory);
    map_pages();
    enable_pse();
    enable_paging();
}
