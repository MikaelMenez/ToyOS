const fb = @import("framebuffer.zig");
comptime {
    @export(&write, .{ .name = "serial_write", .linkage = .strong });
}
pub const COM1 = 0x3F8;

export fn serial_configure_baud_rate(com: u16, divisor: u16) callconv(.c) void {
    fb.outb(com + 3, 0x80);
    fb.outb(com, @intCast((divisor >> 8) & (0x00FF)));
    fb.outb(com, @intCast(divisor & 0x00FF));
}
export fn serial_configure_line(com: u16) callconv(.c) void {
    fb.outb(com + 3, 0x03);
}
pub inline fn inb(port: u16) u8 {
    return asm volatile ("inb %[port], %[result]"
        : [result] "={al}" (-> u8),
        : [port] "{dx}" (port),
    );
}

export fn serial_is_transmit_fifo_empty(com: u32) callconv(.c) i32 {
    const port: u16 = @intCast(com + 5);
    const data = inb(port);
    return @intCast(data & 0x20);
}
pub export fn put_char(com: u32, char: u8) callconv(.c) void {
    while (serial_is_transmit_fifo_empty(com) == 0) {}
    const port: u16 = @intCast(com);
    fb.outb(port, char);
}
pub fn write(text: [*]const u8, len: usize) callconv(.c) c_int {
    for (text[0..len]) |char| {
        put_char(COM1, char);
    }
    return @intCast(len);
}
