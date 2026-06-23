pub const Frame = packed struct(u16) {
    ascii: u8,
    fg: u4,
    bg: u4,
};
const FG = 13;
const BG = 0;
var current_row: usize = 0;
var current_col: usize = 0;
const MAX_ROWS = 25;
const MAX_COLS = 80;
// O endereço do framebuffer VGA text mode
const FB_PTR: [*]Frame = @ptrFromInt(0xB8000);

export fn write_framebuffer(fb: [*]Frame, i: usize, ascii: u8, fg: u8, bg: u8) callconv(.c) void {
    fb[i] = Frame{
        .ascii = ascii,
        .fg = @intCast(fg),
        .bg = @intCast(bg),
    };
}

export fn outb(port: u16, data: u8) callconv(.c) void {
    asm volatile ("outb %[data], %[port]"
        :
        : [port] "{dx}" (port),
          [data] "{al}" (data),
    );
}

export fn move_cursor(position: u16) callconv(.c) void {
    outb(0x3D4, 14);
    outb(0x3D5, @intCast((position >> 8) & 0xFF));

    outb(0x3D4, 15);
    outb(0x3D5, @intCast(position & 0xFF));
}
fn scroll() void {
    var row: usize = 1;
    while (row < MAX_ROWS) : (row += 1) {
        var col: usize = 0;
        while (col < MAX_COLS) : (col += 1) {
            const src_idx = row * MAX_COLS + col;
            const dest_idx = (row - 1) * MAX_COLS + col;
            FB_PTR[dest_idx] = FB_PTR[src_idx];
        }
    }
    var col: usize = 0;
    while (col < MAX_COLS) : (col += 1) {
        const idx = (MAX_ROWS - 1) * MAX_COLS + col;
        write_framebuffer(FB_PTR, idx, ' ', FG, BG);
    }
    current_row = MAX_ROWS - 1;
}
pub export fn write(buffer: [*]const u8, len: usize) callconv(.c) c_int {
    for (buffer[0..len]) |char| {
        if (char == '\n') {
            current_row += 1;
            current_col = 0;
        } else {
            const idx = (current_row * MAX_COLS) + current_col;
            write_framebuffer(FB_PTR, idx, char, FG, BG);
            current_col += 1;
        }
        if (current_col >= MAX_COLS) {
            current_col = 0;
            current_row += 1;
        }
        if (current_row >= MAX_ROWS) {
            scroll();
        }
    }
    const cursor_pos = (current_row * MAX_COLS) + current_col;
    move_cursor(@intCast(cursor_pos));
    return @intCast(len);
}
