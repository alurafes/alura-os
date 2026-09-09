# alura-os

> Simple C x86 kernel

Goals are not defined so far, just experementing with everything. I would definitely like to have some user-space things in the end :)

Things done so far:
* Booting via GRUB (Multiboot)
* Physical memory manager (memory bitmap)
* Virtual memory (Higher half mapping)
* Kernel heap (basic K&R style allocator)
* GDT (Both kernel and user, TSS)
* IDT, IRQ, Exceptions, PIC remapping
* Task switching and scheduling (MLFQ)
* ... (GOTTA WRITE ALL THE THINGS I MADE)
* Userland libc via a ported newlib

## Building

One-time setup: build the `i686-elf` cross compiler to build userland programs. This script builds binutils, gcc and newlib from source and installs them to `~/opt/cross-alura`

```sh
./toolchain/build.sh
```

After that:

```sh
make # build the kernel + initrd tar archive and puts into build/kernel.iso
make run # boot it in QEMU
```