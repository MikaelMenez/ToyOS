// Capítulo 5 - Segmentation
// Monta a Global Descriptor Table (GDT) e chama gdtFlush (implementada em
// gdt_flush.s) para carregá-la no processador via lgdt.

// Um descritor de segmento (8 bytes), conforme o manual da Intel
// vol. 3A, capítulo 3 ("Segment Descriptors").
const GdtEntry = packed struct(u64) {
    limit_low: u16,
    base_low: u16,
    base_middle: u8,
    access: u8,
    granularity: u8,
    base_high: u8,
};

// Struct usada pela instrução lgdt: tamanho da tabela e seu endereço.
const GdtPtr = packed struct(u48) {
    size: u16,
    address: u32,
};

var gdt: [3]GdtEntry = undefined;
var gp: GdtPtr = undefined;

// Implementada em gdt_flush.s: executa lgdt e recarrega os registradores
// de segmento (ds, ss, es, fs, gs via mov, e cs via far jump).
extern fn gdtFlush(gdt_ptr_addr: u32) callconv(.c) void;

fn setGate(num: usize, base: u32, limit: u32, access: u8, gran: u8) void {
    gdt[num] = GdtEntry{
        .limit_low = @truncate(limit & 0xFFFF),
        .base_low = @truncate(base & 0xFFFF),
        .base_middle = @truncate((base >> 16) & 0xFF),
        .access = access,
        .granularity = @truncate(((limit >> 16) & 0x0F) | (gran & 0xF0)),
        .base_high = @truncate((base >> 24) & 0xFF),
    };
}

pub fn gdtInstall() void {
    gp = GdtPtr{
        .size = @truncate(@sizeOf(GdtEntry) * gdt.len - 1),
        .address = @intCast(@intFromPtr(&gdt)),
    };

    // índice 0: null descriptor (obrigatório, nunca usado)
    setGate(0, 0, 0, 0, 0);

    // índice 1, offset 0x08: segmento de código do kernel
    setGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // índice 2, offset 0x10: segmento de dados do kernel
    setGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdtFlush(@intCast(@intFromPtr(&gp)));
}