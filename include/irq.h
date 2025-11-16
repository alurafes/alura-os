#ifndef ALURA_IRQ_H
#define ALURA_IRQ_H

#include <stdint.h>
#include <stddef.h>

#include "idt.h"
#include "pic.h"

#define IRQ_MAX_ENTRIES_COUNT 16

typedef enum irq_result_t {
    IRQ_RESULT_OK,
    IRQ_RESULT_INDEX_ALREADY_IN_USE,
    IRQ_RESULT_OUT_OF_BOUNDS
} irq_result_t;

typedef void (*irq_handler_function)(register_interrupt_data_t*);

typedef struct irq_t {
    irq_handler_function irq_handlers[IRQ_MAX_ENTRIES_COUNT];
} irq_t;

extern irq_t irq;
void irq_module_init();

extern void* irq_stubs[];

void irq_handler(register_interrupt_data_t* data);

irq_result_t irq_register_handler(irq_t* irq, size_t irq_index, irq_handler_function handler);
irq_result_t irq_remove_handler(irq_t* irq, size_t irq_index);

#endif // ALURA_IRQ_H