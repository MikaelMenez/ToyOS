OBJDIR = obj
OBJECTS = $(OBJDIR)/loader.o \
          $(OBJDIR)/kmain.o \
          $(OBJDIR)/gdt.o \
          $(OBJDIR)/gdt_s.o \
          $(OBJDIR)/idt.o \
          $(OBJDIR)/interrupts.o \
          $(OBJDIR)/io.o \
          $(OBJDIR)/fb.o \
          $(OBJDIR)/serial.o \
          $(OBJDIR)/pic.o \
          $(OBJDIR)/pmm.o \
          $(OBJDIR)/usermode.o \
          $(OBJDIR)/enter_usermode.o \
          $(OBJDIR)/vfs.o \
          $(OBJDIR)/tarfs.o \
          $(OBJDIR)/ramfs.o \
          $(OBJDIR)/string.o

CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -I include
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf

all: os.iso

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: kernel/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o: kernel/%.s | $(OBJDIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/%.o: drivers/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o: drivers/%.s | $(OBJDIR)
	$(AS) $(ASFLAGS) $< -o $@

kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

# --- Regra automática para criar o TARFS com múltiplos arquivos e o programa ---
iso/modules/initrd.tar: teste.txt teste2.txt $(USER_BIN)
	mkdir -p iso/modules
	cp iso/modules/program ./program
	tar cf iso/modules/initrd.tar teste.txt teste2.txt program
	rm program

# --- Agora o os.iso depende também do initrd.tar ---
os.iso: kernel.elf iso/modules/program iso/modules/initrd.tar
	cp kernel.elf iso/boot/kernel.elf
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -quiet -boot-info-table -o os.iso iso

run: os.iso
	qemu-system-i386 -cdrom os.iso -serial file:serial.log -monitor stdio

clean:
	rm -rf $(OBJDIR) kernel.elf os.iso serial.log iso/modules

# --- Regras para o Programa de Usuário (Ring 3) ---

USER_DIR = user
USER_C = $(USER_DIR)/program.c
USER_S = $(USER_DIR)/start.s
USER_OBJ = $(OBJDIR)/program_c.o $(OBJDIR)/start_s.o
USER_BIN = iso/modules/program

$(OBJDIR)/program_c.o: $(USER_C) | $(OBJDIR)
	gcc -m32 -ffreestanding -fno-pie -c $< -o $@

$(OBJDIR)/start_s.o: $(USER_S) | $(OBJDIR)
	nasm -f elf32 $< -o $@

$(USER_BIN): $(USER_OBJ) $(USER_DIR)/link.ld
	mkdir -p iso/modules
	ld -m elf_i386 -T $(USER_DIR)/link.ld -o $@ $(USER_OBJ)
