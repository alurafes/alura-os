#ifndef ALURA_PIC_H
#define ALURA_PIC_H

#include "io.h"

#define PIC1 0x20
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)

#define PIC2 0xA0
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define PIC_COMMAND_INITIALIZE 0x11
#define PIC_COMMAND_EOI 0x20

#define PIC_VALUE_HAS_SLAVE_AT_IRQ2 (1 << 2)
#define PIC_VALUE_CASCADE_IRQ2 2
#define PIC_VALUE_8086 0x01

#define PIC1_REMAPPED_VECTOR 0x20
#define PIC2_REMAPPED_VECTOR 0x28

void pic_remap(int offset_master, int offset_slave);
void pic_send_eoi(int irq);
void pic_init_module();
void pic_print_masks();

#endif // ALURA_PIC_H