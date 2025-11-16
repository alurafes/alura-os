#include "irq.h"

#include "print.h"
#include "vga.h"

void irq_handler(register_interrupt_data_t* data)
{
    int irq_index = data->interrupt_index - PIC1_REMAPPED_VECTOR;
    irq_handler_function handler = irq.irq_handlers[irq_index];
    if (handler) handler(data);
    pic_send_eoi(irq_index);
}

void irq_reset_handlers()
{
    for (int i = 0; i < IRQ_MAX_ENTRIES_COUNT; ++i)
    {
        irq.irq_handlers[i] = 0;
    }
}

void irq_set_idt_entries()
{
    for (int i = 0; i < IRQ_MAX_ENTRIES_COUNT; ++i)
    {
        idt_set_entry(&idt, i + IDT_BASE_ENTRIES_COUNT, (uint32_t)irq_stubs[i], 0x08, 0x8E);
    }
}

irq_result_t irq_register_handler(irq_t* irq, size_t irq_index, irq_handler_function handler)
{
    if (irq_index > IRQ_MAX_ENTRIES_COUNT - 1) return IRQ_RESULT_OUT_OF_BOUNDS;
    if (irq->irq_handlers[irq_index]) return IRQ_RESULT_INDEX_ALREADY_IN_USE;
    irq->irq_handlers[irq_index] = handler;
    return IRQ_RESULT_OK;
}

irq_result_t irq_remove_handler(irq_t* irq, size_t irq_index)
{
    if (irq_index > IRQ_MAX_ENTRIES_COUNT - 1) return IRQ_RESULT_OUT_OF_BOUNDS;
    irq->irq_handlers[irq_index] = NULL;
    return IRQ_RESULT_OK;
}

irq_t irq;
void irq_module_init()
{
    irq_reset_handlers();
    irq_set_idt_entries();
}