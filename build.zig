const std = @import("std");
pub fn build(b: *std.Build) void {
    const target = b.resolveTargetQuery(.{
        .cpu_arch = .x86,
        .os_tag = .freestanding,
        .abi = .none,
    });
    const optimize = b.standardOptimizeOption(.{});
    const root_module = b.createModule(.{
        .root_source_file = b.path("kmain.zig"),
        .target = target,
        .optimize = optimize,
    });
    const kernel = b.addExecutable(.{
        .name = "kernel.elf",
        .root_module = root_module,
    });
    kernel.entry = .disabled;
    kernel.setLinkerScript(b.path("link.ld"));

    const nasm = b.addSystemCommand(&.{ "nasm", "-f", "elf32", "loader.s", "-o" });
    const loader_o = nasm.addOutputFileArg("loader.o");
    kernel.root_module.addObjectFile(loader_o);

    const nasm_gdt = b.addSystemCommand(&.{ "nasm", "-f", "elf32", "gdt_flush.s", "-o" });
    const gdt_flush_o = nasm_gdt.addOutputFileArg("gdt_flush.o");
    kernel.root_module.addObjectFile(gdt_flush_o);

    b.installArtifact(kernel);

    const iso_step = b.step("iso", "Gera a imagem ISO bootavel do ToyOS");
    const mkdir_cmd = b.addSystemCommand(&.{ "mkdir", "-p", "iso/boot/grub" });
    iso_step.dependOn(&mkdir_cmd.step);
    const cp_cmd = b.addSystemCommand(&.{"cp"});
    cp_cmd.addArtifactArg(kernel);
    cp_cmd.addArg("iso/boot/kernel.elf");
    cp_cmd.step.dependOn(&kernel.step);
    iso_step.dependOn(&cp_cmd.step);
    const mkisofs_cmd = b.addSystemCommand(&.{
        "mkisofs",
        "-R",
        "-b",
        "boot/grub/stage2_eltorito",
        "-no-emul-boot",
        "-boot-load-size",
        "4",
        "-boot-info-table",
        "-o",
        "os.iso",
        "iso",
    });
    mkisofs_cmd.step.dependOn(&cp_cmd.step);
    iso_step.dependOn(&mkisofs_cmd.step);
    const run_cmd = b.addSystemCommand(&.{
        "qemu-system-i386",
        "-cdrom",
        "os.iso",
        "-serial",
        "stdio",
    });
    const run_step = b.step("run", "Roda o ToyOS no QEMU com log serial");
    run_step.dependOn(&run_cmd.step);
}
