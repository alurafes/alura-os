CC      := gcc
LD      := ld

CFLAGS  := -m32 -ffreestanding -fno-pic -fno-pie -nostartfiles -nodefaultlibs -nostdlib
LDFLAGS := -m elf_i386 -n -T linker.ld

BUILDDIR := build
BINDIR   := initrd/bin

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c,$(BUILDDIR)/%.o,$(SRCS))
ELFS := $(patsubst src/%.c,$(BINDIR)/%.elf,$(SRCS))

all: $(ELFS)

$(BUILDDIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

$(BINDIR)/%.elf: $(BUILDDIR)/%.o
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $< -o $@

clean:
	rm -rf $(BUILDDIR) $(BINDIR)