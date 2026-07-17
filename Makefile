# Pastas
INCLUDE = include
DRIVERS = drivers
KERNEL = kernel

# Arquivos fonte
SOURCES = $(DRIVERS)/fb.c $(DRIVERS)/pic.c $(DRIVERS)/serial.c \
          $(KERNEL)/kmain.c $(KERNEL)/idt.c $(KERNEL)/gdt.c

# Objetos
OBJECTS = loader.o kmain.o fb.o pic.o serial.o gdt.o gdt_s.o idt.o interrupts.o io.o

CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -I$(INCLUDE)
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf

all: kernel.elf

kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

# Regras de compilação
loader.o: $(KERNEL)/loader.s
	$(AS) $(ASFLAGS) $< -o $@

kmain.o: $(KERNEL)/kmain.c
	$(CC) $(CFLAGS) $< -o $@

fb.o: $(DRIVERS)/fb.c
	$(CC) $(CFLAGS) $< -o $@

pic.o: $(DRIVERS)/pic.c
	$(CC) $(CFLAGS) $< -o $@

serial.o: $(DRIVERS)/serial.c
	$(CC) $(CFLAGS) $< -o $@

idt.o: $(KERNEL)/idt.c
	$(CC) $(CFLAGS) $< -o $@

gdt.o: $(KERNEL)/gdt.c
	$(CC) $(CFLAGS) $< -o $@

gdt_s.o: $(KERNEL)/gdt.s
	$(AS) $(ASFLAGS) $< -o $@

interrupts.o: $(KERNEL)/interrupts.s
	$(AS) $(ASFLAGS) $< -o $@

io.o: $(DRIVERS)/io.s
	$(AS) $(ASFLAGS) $< -o $@

run: kernel.elf
	qemu-system-i386 -kernel kernel.elf -serial file:serial.log

clean:
	rm -rf *.o kernel.elf serial.log
