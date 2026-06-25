const fb = @import("utils/framebuffer.zig");
const sp = @import("utils/serial_port.zig");
export fn kmain() callconv(.c) void {
    // Código de teste rápido da seção 4.2.2: escreve o caractere 'A' (0x41)
    // com fundo preto e frente verde (0x02) no endereço físico 0xB8000
    // const vga_mem: [*]volatile u16 = @ptrFromInt(0xB8000);
    //vga_mem[0] = (0x02 << 8) | @as(u16, 0x41);
    const msgfb = "Ola, Mundo! Usando o framebuffer.";

    const msgsp = "Ola, Mundo! Usando a serial port.";
    // 2. A função write espera um ponteiro bruto ([*]const u8).
    // O Zig faz a coerção automática de um ponteiro de array (*const [X]u8)
    // para um ponteiro bruto ([*]const u8) tranquilamente.
    // Passamos também o tamanho usando a propriedade '.len' da string.
    _ = fb.write(msgfb, msgfb.len);
    _ = sp.write(msgsp, msgsp.len);
    // 3. Trava a CPU em um loop infinito para o kernel não dar crash após terminar
    while (true) {
        asm volatile ("hlt");
    }
}
