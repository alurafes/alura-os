#include "idt.h"

#include "print.h"

#include "drivers/vga.h"

#include "syscall.h"

idt_result_t idt_create(idt_t* idt)
{
    idt->pointer.limit = sizeof(idt->entries) - 1;
    idt->pointer.base = (uint32_t)&idt->entries;

    for (int i = 0; i < IDT_BASE_ENTRIES_COUNT; ++i)
    {
        idt_set_entry(idt, i, (uint32_t)isr_stubs[i], 0x08, 0x8E); // 0b10001111 - 1 - present | 00 - kernel | 0 - zero | 1110 - interrupt gate
    }

    idt_set_entry(idt, IDT_SYSCALL, (uint32_t)isr_syscall, 0x08, 0xEE);
    idt_flush(&idt->pointer);
    return IDT_RESULT_OK;
}

idt_result_t idt_set_entry(idt_t* idt, int index, uint32_t offset, uint16_t segment_selector, uint8_t type_attributes)
{
    if (index >= IDT_MAX_ENTRIES_COUNT) return IDT_RESULT_OUT_OF_BOUNDS;

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

void isr_handler(register_interrupt_data_t* data)
{
    if (data->interrupt_index < PIC1_REMAPPED_VECTOR)
    {
        vga_set_color(&vga, (vga_color_t){.background = VGA_COLOR_RED, .foreground = VGA_COLOR_BLACK});
        printf("\n\nException #%x: Error Code: %x\nRegisters:\ngs = %x, fs = %x, es = %x, ds = %x\nedi = %x, esi = %x, ebp = %x, ebx = %x, edx = %x, ecx = %x, eax = %x\neip = %x, cs = %x, eflags = %x, useresp = %x, ss = %x", 
            data->interrupt_index, 
            data->error_code, 
            data->gs, 
            data->fs, 
            data->es, 
            data->ds, 
            data->edi, 
            data->esi, 
            data->ebp, 
            data->ebx,
            data->edx, 
            data->ecx, 
            data->eax, 
            data->eip, 
            data->cs, 
            data->eflags, 
            data->useresp, 
            data->ss);
        for(;;) asm("hlt");
    }
    if (data->interrupt_index == IDT_SYSCALL)
    {
        syscall_handler(data);
    }
}

idt_t idt;
void idt_module_init()
{
    idt_create(&idt);
}