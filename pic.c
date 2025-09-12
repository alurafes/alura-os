#include "pic.h"

#include "print.h"

void pic_remap(int offset_master, int offset_slave)
{
    // tell pics that 3 more commands incoming
    io_outb(PIC1_COMMAND, PIC_COMMAND_INITIALIZE);
    io_wait();
    io_outb(PIC2_COMMAND, PIC_COMMAND_INITIALIZE);
    io_wait();
    // set vector
    io_outb(PIC1_DATA, offset_master);
    io_wait();
    io_outb(PIC2_DATA, offset_slave);
    io_wait();
    // set that there are two pics and we must use cascade
    io_outb(PIC1_DATA, PIC_VALUE_HAS_SLAVE_AT_IRQ2);
    io_wait();
    io_outb(PIC2_DATA, PIC_VALUE_CASCADE_IRQ2);
    io_wait();
    // set 8086 mode
    io_outb(PIC1_DATA, PIC_VALUE_8086);
    io_wait();
    io_outb(PIC2_DATA, PIC_VALUE_8086);
    io_wait();
    // unmask
    io_outb(PIC1_DATA, 0);
    io_wait();
	io_outb(PIC2_DATA, 0);
    io_wait();
}

void pic_init_module()
{
    pic_remap(PIC1_REMAPPED_VECTOR, PIC2_REMAPPED_VECTOR);
}

void pic_send_eoi(int irq)
{
    if (irq >= 8) io_outb(PIC2_COMMAND, PIC_COMMAND_EOI);
    io_outb(PIC1_COMMAND, PIC_COMMAND_EOI);
}

void pic_print_masks()
{
    uint8_t m1 = io_inb(PIC1_DATA);
    uint8_t m2 = io_inb(PIC2_DATA);
    printf("PIC masks: master=%x slave=%x\n", m1, m2);
}