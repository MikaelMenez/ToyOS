OBJECTS = kernel/loader.o kernel/kmain.o kernel/gdt.o kernel/gdt_s.o kernel/idt.o kernel/interrupts.o drivers/io.o drivers/fb.o drivers/serial.o drivers/pic.o

CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -I include
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf

all: os.iso

# Regra para criar a pasta e compilar o módulo program
iso/modules/program: program.s
	mkdir -p iso/modules
	$(AS) -f bin program.s -o iso/modules/program

# Regras de compilação do Kernel
kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) $< -o $@

drivers/%.o: drivers/%.c
	$(CC) $(CFLAGS) $< -o $@

kernel/%.o: kernel/%.s
	$(AS) $(ASFLAGS) $< -o $@

kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

# A ISO depende do kernel e do módulo estarem prontos
os.iso: kernel.elf iso/modules/program
	cp kernel.elf iso/boot/kernel.elf
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -quiet -boot-info-table -o os.iso iso

run: os.iso
	qemu-system-i386 -cdrom os.iso -serial file:serial.log -monitor stdio

clean:
	rm -f kernel/*.o drivers/*.o kernel.elf os.iso serial.log
	rm -rf iso/modules
