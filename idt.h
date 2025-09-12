#ifndef ALURA_IDT_H
#define ALURA_IDT_H

#include <stdint.h>

#include "pic.h"

#define IDT_ENTRIES_COUNT 256

typedef enum idt_result_t {
    IDT_RESULT_OK,
    IDT_RESULT_OUT_OF_BOUNDS
} idt_result_t;

typedef struct idt_entry_t {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t type_attributes;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct idt_pointer_t {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_pointer_t;

typedef struct idt_t {
    idt_entry_t entries[IDT_ENTRIES_COUNT];
    idt_pointer_t pointer;
} idt_t;

idt_result_t idt_create(idt_t* idt);
idt_result_t idt_set_entry(idt_t* idt, int entry, uint32_t offset, uint16_t segment_selector, uint8_t type_attributes);
void idt_flush(idt_pointer_t* pointer);

extern idt_t idt;
void idt_module_init();

extern void* isr_stubs[];

typedef struct isr_interrupt_data {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t interrupt_index, error_code;
    uint32_t eip, cs, eflags, useresp, ss;
} __attribute__((packed)) isr_interrupt_data;

void isr_handler(isr_interrupt_data* data);

#endif // ALURA_IDT_H