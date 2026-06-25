const fb = @import("framebuffer.zig");
export fn serial_configure_baud_rate(com: u16, divisor: u16) callconv(.c) void {
    fb.outb(com + 3, 0x80);
    fb.outb(com, (divisor >> 8) & (0x00FF));
    fb.outb(com, divisor & 0x00FF);
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

export fn serial_is_transmit_fifo_empty(com: u32) i32 {
    return inb((com + 5) & 0x20);
}
