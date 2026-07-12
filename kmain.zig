const fb = @import("utils/framebuffer.zig");

const sp = @import("utils/serial_port.zig");

const gdt = @import("utils/gdt.zig");

const mb = @import("utils/multiboot.zig");
export fn kmain(ebx: usize) callconv(.c) void {
    gdt.gdtInstall();

    const mbinfo: *const mb.MultibootInfo = @ptrFromInt(ebx);

    // 1. Verificamos se há exatamente 1 módulo carregado pelo GRUB
    if (mbinfo.mods_count == 1) {
        const addr_of_module = mbinfo.mods_addr;

        // 2. Definimos o tipo e o chamamos
        const CallModule = *const fn () callconv(.c) void;
        const start_program: CallModule = @ptrFromInt(addr_of_module);

        // 3. Executamos o programa
        start_program();
    }

    // O código abaixo só será executado se o módulo retornar (o que geralmente não acontece)
    const msgfb = "Modulo encerrado ou nao encontrado.";
    _ = fb.write(msgfb, msgfb.len);

    while (true) {
        asm volatile ("hlt");
    }
}
