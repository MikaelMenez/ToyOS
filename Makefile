# ============================================================================
# Makefile do ToyOS
# ----------------------------------------------------------------------------
# Alvos principais:
#   make          → compila tudo e gera a os.iso
#   make run      → compila (se preciso) e abre o QEMU com o sistema
#   make clean    → apaga tudo que foi gerado (build limpo)
#
# Ferramentas necessárias: gcc (-m32), nasm, ld, genisoimage e QEMU.
# Veja o README.md pra saber como instalar cada uma.
# ============================================================================

# ==========================================
# Variáveis do Programa de Usuário (Ring 3)
# ==========================================
USER_DIR = user
USER_C = $(USER_DIR)/shell.c
USER_S = $(USER_DIR)/start.s
USER_OBJ = $(OBJDIR)/program_c.o $(OBJDIR)/start_s.o
USER_BIN = iso/modules/program   # binário que o GRUB entrega pro kernel

OBJDIR = obj
# A lista de tudo que vira o kernel. Cada ".o" é gerado a partir do
# arquivo .c/.s de mesmo nome em kernel/ ou drivers/.
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
          $(OBJDIR)/syscall.o \
          $(OBJDIR)/syscall_s.o \
          $(OBJDIR)/string.o

# Compilador e flags. Repare que o kernel vive SEM a libc do sistema:
# -nostdlib etc. — ele é o sistema.
CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -I include
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf

# Tudo vai terminar na ISO; é só rodar pra começar.
all: os.iso

# Cria a pasta dos objetos caso ainda não exista
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Regras que transformam cada .c/.s do kernel e dos drivers em .o
$(OBJDIR)/%.o: kernel/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o: kernel/%.s | $(OBJDIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/%.o: drivers/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o: drivers/%.s | $(OBJDIR)
	$(AS) $(ASFLAGS) $< -o $@

# Liga todos os objetos do kernel, seguindo o layout do link.ld
kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

# --- Regra limpa para criar o TARFS com múltiplos arquivos e o programa ---
# Cria o "disco" initrd.tar juntando os arquivos de teste e o programa
iso/modules/initrd.tar: teste.txt teste2.txt $(USER_BIN)
	tar cf iso/modules/initrd.tar teste.txt teste2.txt -C iso/modules program

# --- O os.iso depende do kernel, do binário de usuário e do initrd.tar ---
# Copia o kernel pra pasta da ISO e gera o CD (El Torito) com o GRUB
os.iso: kernel.elf $(USER_BIN) iso/modules/initrd.tar
	cp kernel.elf iso/boot/kernel.elf
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -quiet -boot-info-table -o os.iso iso

# Roda o sistema numa VM: o vídeo vai pra tela do QEMU e o log serial
# vai pro arquivo serial.log. O monitor do QEMU fica no terminal.
run: os.iso
	qemu-system-i386 -cdrom os.iso -serial file:serial.log -monitor stdio

# Apaga tudo que é artefato gerado (build limpo)
clean:
	rm -rf $(OBJDIR) kernel.elf os.iso serial.log iso/modules

# --- Regras para compilar o Programa de Usuário (Ring 3) ---
# Um kernel não roda nada "cruzado", então o usuário é compilado à parte,
# também em 32 bits, porém sem as flags restritivas do kernel.
$(OBJDIR)/program_c.o: $(USER_C) | $(OBJDIR)
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -I include -c $< -o $@

$(OBJDIR)/start_s.o: $(USER_S) | $(OBJDIR)
	nasm -f elf32 $< -o $@

# Liga o binário do usuário de forma "crua" (sem cabeçalho), no layout do
# user/link.ld, e guarda dentro de iso/modules pra virar módulo do GRUB.
$(USER_BIN): $(USER_OBJ) $(USER_DIR)/link.ld
	mkdir -p iso/modules
	ld -m elf_i386 -T $(USER_DIR)/link.ld -o $@ $(USER_OBJ)