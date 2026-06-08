all: image-viewer.iso

kernel.bin: kernel.c boot.asm linker.ld
	gcc -m32 -c kernel.c -o kernel.o -ffreestanding -O2 -Wall -Wextra
	nasm -f elf32 boot.asm -o boot.o
	ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o -nostdlib

image-viewer.iso: kernel.bin sky.bmp grub.cfg
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/
	cp sky.bmp iso/
	cp grub.cfg iso/boot/grub/
	grub-mkrescue -o image-viewer.iso iso/ 2>/dev/null || \
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot \
		-boot-load-size 4 -boot-info-table -o image-viewer.iso iso/

clean:
	rm -rf *.o *.bin *.iso iso/

.PHONY: all clean
