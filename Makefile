boot.o: boot.s
	nasm -f elf32 boot.s -o build/boot.o

kernel.o: kernel.c
	gcc -m32 -c kernel.c -o build/kernel.o -ffreestanding

link: linker.ld boot.o kernel.o
	ld -m elf_i386 -n -T linker.ld -o build/iso/boot/kernel.elf build/boot.o build/kernel.o

iso: link
	grub2-mkrescue -o build/kernel.iso build/iso

build-run: iso
	qemu-system-x86_64 -cdrom build/kernel.iso