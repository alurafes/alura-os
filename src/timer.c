#include "timer.h"

uint64_t timer_ticks = 0;

void timer_set_phase()
{
    uint32_t divisor = PIT_CLOCK / TIMER_PHASE;
    io_outb(PIT_COMMAND, TIMER_CHANNEL_0 | TIMER_READ_WRITE_HIGH_LOW | TIMER_MODE_SQUARE_WAVE | TIMER_16_BIT_BINARY);
    io_outb(PIT0, divisor & 0xFF);
    io_outb(PIT0, divisor >> 8);
}

void timer_irq_handler(register_interrupt_data_t* data)
{
    timer_ticks++;
}

void timer_module_init()
{
    timer_set_phase();
    irq_register_handler(&irq, 0, timer_irq_handler);
}