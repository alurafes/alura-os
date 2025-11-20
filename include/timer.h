#ifndef ALURA_TIMER_H
#define ALURA_TIMER_H

#include "irq.h"
#include "io.h"
#include "task_manager.h"

 
#define TIMER_PHASE 1000

#define PIT_CLOCK 1193180

#define PIT0 0x40
#define PIT1 0x41
#define PIT2 0x42
#define PIT_COMMAND 0x43

#define TIMER_CHANNEL_0 0x0
#define TIMER_CHANNEL_1 0x40
#define TIMER_CHANNEL_2 0xC0
#define TIMER_READ_WRITE_HIGH_LOW 0x30
#define TIMER_MODE_SQUARE_WAVE 0x6
#define TIMER_16_BIT_BINARY 0x0

void timer_set_phase();
void timer_module_init();

#endif // ALURA_TIMER_H