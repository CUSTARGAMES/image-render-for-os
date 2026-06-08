all: image-viewer.iso

kernel.bin: kernel.c boot.asm linker.ld
	gcc -m32 -c kernel.c -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector
	nasm -f elf32 boot.asm -o boot.o
	ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o -nostdlib

image-viewer.iso: kernel.bin sky.bmp grub.cfg
	@echo "Building bootable ISO..."
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/
	cp sky.bmp iso/
	cp grub.cfg iso/boot/grub/
	grub-mkrescue -o image-viewer.iso iso/
	rm -rf iso

clean:
	rm -rf *.o *.bin *.iso iso/

.PHONY: all clean
