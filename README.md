# Capítulo 2 - Bootloader

## Objetivos

O objetivo deste capítulo consiste em implementar a primeira etapa de inicialização do sistema operacional, criando o código inicial em Assembly e configurando o GNU GRUB para realizar o carregamento do sistema.

Nesta etapa, o foco está em entender como o computador passa pelo processo inicial de boot até a execução do código desenvolvido pelo sistema operacional.

---

## Funcionamento

Ao iniciar o computador, a BIOS/UEFI realiza a inicialização básica do hardware e procura um bootloader.

O GNU GRUB é responsável por carregar o código inicial do sistema. Para isso, ele utiliza suas configurações definidas no arquivo `menu.lst`.

O arquivo Assembly `loader.s` representa o primeiro código desenvolvido pelo sistema, sendo responsável pela preparação inicial do ambiente e pela transferência da execução para as próximas etapas do sistema operacional.

---

## Arquivos

### loader.s

Código escrito em Assembly responsável pela inicialização do sistema.

**Funções:**

- Executar as primeiras instruções do sistema;
- Preparar o ambiente inicial;
- Permitir a transição para o kernel.

---

### menu.lst

Arquivo de configuração do GNU GRUB.

**Funções:**

- Informar ao GRUB qual arquivo deve ser carregado;
- Definir as configurações de inicialização do sistema.

---

## Ferramentas utilizadas

- **NASM:** utilizado para transformar o código Assembly em código de máquina.

- **GNU GRUB:** utilizado como bootloader para carregar o sistema.

- **QEMU:** utilizado para testar a inicialização do sistema em uma máquina virtual.

---

## Execução

Após configurar os arquivos, o projeto é compilado para gerar uma imagem inicializável.

A imagem é executada pelo QEMU, que simula o funcionamento de um computador real e permite testar o processo de boot.
