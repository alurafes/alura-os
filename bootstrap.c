#include "memory_paging.h"

#define KERNEL_PHYSICAL 0x00100000
#define KERNEL_VIRTUAL 0xC0000000

static uint32_t __attribute__((section(".bootstrap.data"), aligned(4096))) bootstrap_page_directory[1024];
static uint32_t __attribute__((section(".bootstrap.data"), aligned(4096))) low_page_table[1024]; // just enough for first 4mb. i think my kernel will never grow higher than that :D
static uint32_t __attribute__((section(".bootstrap.data"), aligned(4096))) high_page_table[1024];

__attribute__((section(".bootstrap.text")))
static void reset_arrays()
{
    for (int i = 0; i < 1024; i++) {
        bootstrap_page_directory[i] = 0;
        low_page_table[i] = 0;
        high_page_table[i] = 0;
    }
}

__attribute__((section(".bootstrap.text")))
static void identity_map_first_four_megabytes()
{
    for (int i = 0; i < 1024; ++i)
    {
        low_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_READ_WRITE;
    }
    bootstrap_page_directory[0] = (uint32_t)low_page_table | PAGE_PRESENT | PAGE_READ_WRITE;
}

__attribute__((section(".bootstrap.text")))
static void map_higher_half_kernel()
{
    for (int i = 0; i < 1023; ++i)
    {
        high_page_table[i] = ((i * 0x1000) + KERNEL_PHYSICAL) | PAGE_PRESENT | PAGE_READ_WRITE;
    }
    bootstrap_page_directory[KERNEL_VIRTUAL >> 22] = (uint32_t)high_page_table | PAGE_PRESENT | PAGE_READ_WRITE;
    high_page_table[1023] = 0x000B8000 | PAGE_PRESENT | PAGE_READ_WRITE;
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
    reset_arrays();
    identity_map_first_four_megabytes();
    map_higher_half_kernel();
    enable_paging();
}
