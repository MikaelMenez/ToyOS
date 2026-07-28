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
		  $(OBJDIR)/enter_usermode.o

CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -I include
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf

all: os.iso

$(OBJDIR):
	mkdir -p $(OBJDIR)

iso/modules/program: program.s
	mkdir -p iso/modules
	$(AS) -f bin program.s -o iso/modules/program

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

os.iso: kernel.elf iso/modules/program
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

$(OBJDIR)/program_c.o: $(USER_C)
	gcc -m32 -ffreestanding -fno-pie -c $< -o $@

$(OBJDIR)/start_s.o: $(USER_S)
	nasm -f elf32 $< -o $@

$(USER_BIN): $(USER_OBJ) $(USER_DIR)/link.ld
	ld -m elf_i386 -T $(USER_DIR)/link.ld -o $@ $(USER_OBJ)