#include "gdt.h"

gdt_result_t gdt_create(gdt_t* gdt)
{
    gdt->pointer.limit = sizeof(gdt->entries) - 1;
    gdt->pointer.base = (uint32_t)&gdt->entries;

    gdt_set_entry(gdt, 0, 0, 0, 0, 0); // Null
    gdt_set_entry(gdt, 1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel Code 0b10011010 - Present: 1 | Kernel: 00 | S: 1 | Executable: 1 | Clear: 0, Read: 1, A: 0
    gdt_set_entry(gdt, 2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel Data 0b10010010 - Present: 1 | Kernel: 00 | S: 1 | Executable: 0 | Grows Up: 0, Write: 1, A: 0
    gdt_set_entry(gdt, 3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User Code 0b11111010 - Present: 1 | Kernel: 00 | S: 1 | Executable: 1 | Clear: 0, Read: 1, A: 0
    gdt_set_entry(gdt, 4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User Data 0b11110010 - Present: 1 | Kernel: 00 | S: 1 | Executable: 0 | Grows Up: 0, Write: 1, A: 0

    gdt_flush((uint32_t)&gdt->pointer);
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