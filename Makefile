# Tools

CC      := gcc
AS      := nasm
LD      := ld
GRUB    := grub2-mkrescue
QEMU    := qemu-system-x86_64

# Build Flags

CFLAGS  := -m32 -ffreestanding -c
ASFLAGS := -f elf32
LDFLAGS := -m elf_i386 -n -T linker.ld

# Outputs

BUILDDIR := build
ISODIR   := $(BUILDDIR)/iso/boot

# Sources

C_SOURCES := kernel.c vga.c gdt.c terminal.c print.c idt.c
ASM_SOURCES := boot.s

# Objects

C_OBJS   := $(C_SOURCES:%.c=$(BUILDDIR)/%.o)
ASM_OBJS := $(ASM_SOURCES:%.s=$(BUILDDIR)/%.o)
OBJS     := $(C_OBJS) $(ASM_OBJS)

# Targets

all: iso

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

link: $(OBJS)
	$(LD) $(LDFLAGS) -o $(ISODIR)/kernel.elf $(OBJS)

iso: link
	$(GRUB) -o $(BUILDDIR)/kernel.iso $(BUILDDIR)/iso

run: iso
	$(QEMU) -cdrom $(BUILDDIR)/kernel.iso -s -S

clean:
	rm -rf $(BUILDDIR)/*.o $(ISODIR)/kernel.elf $(BUILDDIR)/kernel.iso