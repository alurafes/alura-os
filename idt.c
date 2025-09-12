#include "idt.h"

idt_result_t idt_create(idt_t* idt)
{
    idt->pointer.limit = sizeof(idt->entries) - 1;
    idt->pointer.base = (uint32_t)&idt->entries;

    // todo: actually create interrupt handlers

    idt_flush(&idt->pointer);
    return IDT_RESULT_OK;
}

idt_result_t idt_set_entry(idt_t* idt, int index, uint32_t offset, uint16_t segment_selector, uint8_t type_attributes)
{
    if (index >= IDT_ENTRIES_COUNT) return IDT_RESULT_OUT_OF_BOUNDS;

    idt->entries[index].offset_low = offset & 0xFFFF;
    idt->entries[index].offset_high = (offset >> 16) & 0xFFFF;
    idt->entries[index].reserved = 0;
    idt->entries[index].segment_selector = segment_selector;
    idt->entries[index].type_attributes = type_attributes;

    return IDT_RESULT_OK;
}

void idt_flush(idt_pointer_t* pointer)
{
    __asm__ volatile ("lidt (%0)" : : "r"(pointer));
}

idt_t idt;
void idt_module_init()
{
    idt_create(&idt);
}