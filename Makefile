# Tools

CC      := gcc
AS      := nasm
LD      := ld
GRUB    := grub2-mkrescue
QEMU    := qemu-system-i386

# Build Flags

CFLAGS  := -m32 -ffreestanding -c -g -Wall -Wextra
ASFLAGS := -f elf32
LDFLAGS := -m elf_i386 -n -T linker.ld

# Outputs

BUILDDIR := build
ISODIR   := $(BUILDDIR)/iso/boot

# Sources

C_SOURCES := $(wildcard src/*.c)
ASM_SOURCES := $(wildcard src/*.s)

# Objects

C_OBJS   := $(C_SOURCES:src/%.c=$(BUILDDIR)/%.o)
ASM_OBJS := $(ASM_SOURCES:src/%.s=$(BUILDDIR)/%.o)
OBJS     := $(C_OBJS) $(ASM_OBJS)

# Targets

all: iso

$(BUILDDIR)/%.o: src/%.c
	$(CC) -Iinclude $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: src/%.s
	$(AS) $(ASFLAGS) $< -o $@

link: $(OBJS)
	$(LD) $(LDFLAGS) -o $(ISODIR)/kernel.elf $(OBJS)

iso: link
	$(GRUB) -o $(BUILDDIR)/kernel.iso $(BUILDDIR)/iso

run: iso
	$(QEMU) -cdrom $(BUILDDIR)/kernel.iso -s -S -m 256M -d int

clean:
	rm -rf $(BUILDDIR)/*.o $(ISODIR)/kernel.elf $(BUILDDIR)/kernel.iso