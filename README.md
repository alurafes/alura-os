# alura-os

> Simple C x86 kernel

Goals are not defined so far, just experementing with everything. I would definitely like to have some user-space things in the end :)

Things done so far:
* Booting via GRUB (Multiboot)
* Physical memory manager (memory bitmap)
* Virtual memory (Higher half mapping)
* Kernel heap (basic K&R style allocator)
* GDT (just ring 0 entries for now)
* IDT, IRQ, Exceptions, PIC remapping