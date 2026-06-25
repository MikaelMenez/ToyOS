
# ToyOS

Um projeto de sistema operacional minimalista (bare-metal) baseado no livro *The Little OS Book*, desenvolvido em **Zig (0.16.0)** e **Assembly NASM** para a arquitetura x86 (32 bits), utilizando o bootloader **GRUB Legacy** via Multiboot.



## 📋 Pré-requisitos (Ubuntu)

Para configurar o ambiente no Ubuntu, instale as ferramentas necessárias executando o comando abaixo:

```bash
sudo apt-get update && sudo apt-get install build-essential nasm genisoimage qemu-system-x86

```

### Instalação do Zig 0.16.0

Como a versão 0.16.0 é a versão de desenvolvimento atual (*master/nightly*), você deve baixá-la diretamente dos servidores oficiais do Zig. Execute a sequência abaixo para baixar, extrair e adicionar o compilador ao seu `PATH`:

```bash
# Baixa o binário oficial do Zig 0.16.0 para Linux x86_64
wget [https://ziglang.org/builds/zig-linux-x86_64-0.16.0-dev.643+7a1b3c4d5.tar.xz](https://ziglang.org/builds/zig-linux-x86_64-0.16.0-dev.643+7a1b3c4d5.tar.xz)

# Extrai o arquivo
tar -xf zig-linux-x86_64-0.16.0-dev.643+7a1b3c4d5.tar.xz

# Move para o diretório local de binários
sudo mv zig-linux-x86_64-0.16.0-dev.643+7a1b3c4d5 /opt/zig

# Adiciona o Zig ao PATH (adicione esta linha ao seu ~/.bashrc ou ~/.zshrc para tornar permanente)
export PATH=/opt/zig:$PATH

```

---

## 🛠️ Compilação e Execução

Para compilar o projeto, estruturar a árvore do GRUB e gerar o arquivo de imagem final:

```bash
zig build iso

```

Para rodar a ISO gerada no emulador QEMU:

```bash
zig build run

```

---

## 🗺️ Linha do Tempo do Projeto (Baseada no Livro)

A evolução do repositório segue a ordem dos conceitos teóricos apresentados nos capítulos de arquitetura de sistemas operacionais:

### 📑 Capítulo 2: O Ambiente de Desenvolvimento e o Boot

* **Foco do Livro**: Compreender o processo de boot do PC, a transição do BIOS para o Bootloader e o protocolo Multiboot.
* **Implementação**:
* Criação do arquivo `loader.s` para estruturar os magic numbers do cabeçalho Multiboot v1.
* Configuração da pilha de execução (`esp`) alocando 4KB de memória na seção reservada `.bss` antes de transferir o controle.
* Configuração dos arquivos estáticos do GRUB Legacy (`stage2_eltorito` e `menu.lst`).



### 📑 Capítulo 3: Introdução ao C (Adaptado para Zig) e Linkagem

* **Foco do Livro**: Invocar código de alto nível a partir do Assembly, gerenciar o ponto de entrada e organizar as seções binárias na memória RAM.
* **Implementação**:
* Criação do arquivo `link.ld` definindo o endereço base de carregamento em `0x00100000` (1MB).
* Alinhamento estrito de página de **4KB (`ALIGN(4K)`)** para as seções `.text`, `.rodata`, `.data` e `.bss`, garantindo que o cabeçalho do GRUB não mude de lugar.
* Escrita da função principal `kmain` exposta globalmente para receber o salto do Assembly.



### 📑 Capítulo 4: Saída de Vídeo (O Framebuffer)

* **Foco do Livro**: Interagir diretamente com a memória mapeada de hardware (Memory-Mapped I/O) para escrever texto na tela via console VGA.
* **Implementação**:
* Criação do driver inicial do Framebuffer apontando para o endereço de memória física `0x000B8000`.
* Manipulação direta de ponteiros para escrever caracteres e definir cores de fundo e texto na tela preta do terminal.

