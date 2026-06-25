const std = @import("std");

pub fn build(b: *std.Build) void {
    // 1. Define o alvo como x86 freestanding
    const target = b.resolveTargetQuery(.{
        .cpu_arch = .x86,
        .os_tag = .freestanding,
        .abi = .none,
    });

    const optimize = b.standardOptimizeOption(.{});

    // 2. Cria o módulo raiz (Sintaxe compatível com 0.16.0)
    // 2. Cria o módulo raiz limpo
    const root_module = b.createModule(.{
        .root_source_file = b.path("kmain.zig"),
        .target = target,
        .optimize = optimize,
    });

    // 3. Configura o executável do Kernel
    const kernel = b.addExecutable(.{
        .name = "kernel.elf",
        .root_module = root_module,
    });

    // CORREÇÃO PARA A 0.16.0: Define o entrypoint diretamente na instância
    kernel.entry = .disabled;

    // Vincula o script de linkagem personalizado
    kernel.setLinkerScript(b.path("link.ld"));

    // 4. Monta o loader.s usando o NASM via comando de sistema
    const nasm = b.addSystemCommand(&.{ "nasm", "-f", "elf32", "loader.s", "-o" });
    const loader_o = nasm.addOutputFileArg("loader.o");

    // CORREÇÃO PARA A 0.16.0: O objeto entra no root_module do kernel, não no kernel direto!
    kernel.root_module.addObjectFile(loader_o);
    // Garante que o kernel seja gerado na pasta padrão zig-out/bin/
    b.installArtifact(kernel);

    // 5. Cria o passo customizado 'zig build iso'
    const iso_step = b.step("iso", "Gera a imagem ISO bootavel do ToyOS");

    // Comando para garantir que a pasta do GRUB existe
    const mkdir_cmd = b.addSystemCommand(&.{ "mkdir", "-p", "iso/boot/grub" });
    iso_step.dependOn(&mkdir_cmd.step);

    // Comando que copia o kernel recem-compilado para dentro da pasta iso/boot/
    const cp_cmd = b.addSystemCommand(&.{"cp"});
    cp_cmd.addArtifactArg(kernel);
    cp_cmd.addArg("iso/boot/kernel.elf");

    // Na 0.16.0, dependências de compilação usam o objeto .step diretamente
    cp_cmd.step.dependOn(&kernel.step);
    iso_step.dependOn(&cp_cmd.step);

    // Comando do mkisofs (cdrtools) para gerar o arquivo os.iso final
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
        "file:com1.out", // ou "stdio" se preferir no terminal
    });

    const run_step = b.step("run", "Roda o ToyOS no QEMU com log serial");
    run_step.dependOn(&run_cmd.step);
}
