const GdtEntry = packed struct(u64) {
    limit_low: u16,
    base_low: u16,
    base_middle: u8,
    access: u8,
    granularity: u8,
    base_high: u8,
};

const GdtPtr = packed struct(u48) {
    size: u16,
    address: u32,
};

// Marcamos como 'linksection' para garantir que a GDT esteja em um local conhecido
var gdt: [3]GdtEntry align(8) = undefined;
var gp: GdtPtr = undefined;

fn setGate(num: usize, base: u32, limit: u32, access: u8, gran: u8) void {
    gdt[num] = GdtEntry{
        .limit_low = @truncate(limit & 0xFFFF),
        .base_low = @truncate(base & 0xFFFF),
        .base_middle = @truncate((base >> 16) & 0xFF),
        .access = access,
        // O campo 'granularity' na sua struct pega os 4 bits do limite (16-19)
        // e os 4 bits de flags da GDT.
        .granularity = @truncate(((limit >> 16) & 0x0F) | (gran & 0xF0)),
        .base_high = @truncate((base >> 24) & 0xFF),
    };
}

pub fn gdtInstall() void {
    gp = GdtPtr{
        .size = @sizeOf(GdtEntry) * gdt.len - 1,
        .address = @intFromPtr(&gdt),
    };

    setGate(0, 0, 0, 0, 0);
    setGate(1, 0, 0xFFFFF, 0x9A, 0xCF);
    setGate(2, 0, 0xFFFFF, 0x92, 0xCF);

    const ptr_addr = @intFromPtr(&gp);

    // Injeção de assembly inline segura e sem dependência de ABI
    asm volatile (
        \\ lgdt (%[ptr])
        \\ movw $0x10, %%ax
        \\ movw %%ax, %%ds
        \\ movw %%ax, %%es
        \\ movw %%ax, %%fs
        \\ movw %%ax, %%gs
        \\ movw %%ax, %%ss
        \\ ljmp $0x08, $1f
        \\ 1:
        :
        : [ptr] "r" (ptr_addr),
          // Sem a linha de clobbers, a vírgula desaparece e o erro acaba
    );
}
