#ifndef ALURA_IDT_H
#define ALURA_IDT_H

#include <stdint.h>

#include "pic.h"

#define IDT_MAX_ENTRIES_COUNT 256
#define IDT_BASE_ENTRIES_COUNT 32
#define IDT_SYSCALL 0x80

typedef enum idt_result_t {
    IDT_RESULT_OK = 0,
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
    idt_entry_t entries[IDT_MAX_ENTRIES_COUNT];
    idt_pointer_t pointer;
} idt_t;

idt_result_t idt_create(idt_t* idt);
idt_result_t idt_set_entry(idt_t* idt, int entry, uint32_t offset, uint16_t segment_selector, uint8_t type_attributes);
void idt_flush(idt_pointer_t* pointer);

extern idt_t idt;
void idt_module_init();

extern void* isr_stubs[];
extern void isr_syscall(void);

typedef struct __attribute__((packed))
{
    /* segment registers (pushed last by isr_stub_handler, so lowest addr) */
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
 
    /* general purpose (no esp field - it is no longer pushed) */
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
 
    /* pushed by isr_stubs.s */
    uint32_t interrupt_index;
    uint32_t error_code;
 
    /* pushed by CPU */
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
 
    /* only valid on privilege-level change (ring 3 -> ring 0) */
    uint32_t useresp;
    uint32_t ss;
} register_interrupt_data_t;

void isr_handler(register_interrupt_data_t* data);

#endif // ALURA_IDT_H