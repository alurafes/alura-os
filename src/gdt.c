#include "gdt.h"

gdt_result_t gdt_create(gdt_t* gdt)
{
    gdt->pointer.limit = sizeof(gdt->entries) - 1;
    gdt->pointer.base = (uint32_t)&gdt->entries;

    gdt_set_entry(gdt, GDT_NULL, 0, 0, 0, 0); // Null
    gdt_set_entry(gdt, GDT_KERNEL_CODE, 0, 0xFFFFFFFF, GDT_FLAG_PRESENT | GDT_FLAG_RING0 | GDT_FLAG_SEGMENT | GDT_FLAG_EXECUTABLE | GDT_FLAG_READ_WRITE, GDT_GRANULARITY_4K | GDT_GRANULARITY_32BIT);
    gdt_set_entry(gdt, GDT_KERNEL_DATA, 0, 0xFFFFFFFF, GDT_FLAG_PRESENT | GDT_FLAG_RING0 | GDT_FLAG_SEGMENT | GDT_FLAG_READ_WRITE, GDT_GRANULARITY_4K | GDT_GRANULARITY_32BIT); // Kernel Data 0b10010010 - Present: 1 | Kernel: 00 | S: 1 | Executable: 0 | Grows Up: 0, Write: 1, A: 0
    gdt_set_entry(gdt, GDT_USER_CODE, 0, 0xFFFFFFFF, GDT_FLAG_PRESENT | GDT_FLAG_RING3 | GDT_FLAG_SEGMENT | GDT_FLAG_EXECUTABLE | GDT_FLAG_READ_WRITE, GDT_GRANULARITY_4K | GDT_GRANULARITY_32BIT);
    gdt_set_entry(gdt, GDT_USER_DATA, 0, 0xFFFFFFFF, GDT_FLAG_PRESENT | GDT_FLAG_RING3 | GDT_FLAG_SEGMENT | GDT_FLAG_READ_WRITE, GDT_GRANULARITY_4K | GDT_GRANULARITY_32BIT); // Kernel Data 0b10010010 - Present: 1 | Kernel: 00 | S: 1 | Executable: 0 | Grows Up: 0, Write: 1, A: 0
    gdt_set_entry(gdt, GDT_TSS, (uint32_t)&tss, sizeof(tss) - 1, GDT_FLAG_PRESENT | GDT_FLAG_RING0 | GDT_FLAG_SYSTEM | GDT_FLAG_TSS, GDT_GRANULARITY_BYTE);

    gdt_flush(&gdt->pointer);
    return GDT_RESULT_OK;
}

gdt_result_t gdt_set_entry(gdt_t* gdt, int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity)
{
    if (index >= GDT_ENTRIES_COUNT) return GDT_RESULT_OUT_OF_BOUNDS;

    gdt->entries[index].base_low = base & 0xFFFF;
    gdt->entries[index].base_middle = (base >> 16) & 0xFF;
    gdt->entries[index].base_high = (base >> 24) & 0xFF;

    gdt->entries[index].limit_low = limit & 0xFFFF;
    gdt->entries[index].granularity = (limit >> 16) & 0x0F;
    gdt->entries[index].granularity |= granularity & 0xF0;

    gdt->entries[index].access = access;
    return GDT_RESULT_OK;
}

void gdt_flush(gdt_pointer_t* pointer)
{
    __asm__ volatile (
        "lgdt (%0)\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        "ljmp $0x08, $1f\n\t"
        "1:\n\t"
        :
        : "r"(pointer)
        : "ax", "memory"
    );
}

gdt_t gdt;
void gdt_module_init()
{
    gdt_create(&gdt);
}