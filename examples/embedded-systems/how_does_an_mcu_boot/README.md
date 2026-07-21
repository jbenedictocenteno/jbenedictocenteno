# Minimal STM32U575 boot example

The bare-metal firmware built by hand in the note
[How does an MCU get from reset to `main()`?](../../../docs/notes/embedded-systems/how_does_an_mcu_boot.md).

Three files, no CMSIS and no HAL:

- **`main.c`** — a trivial `main()` with one `.data` global (`counter = 7`) and one `.bss` global (`flag`), used as probes into the boot process.
- **`startup.c`** — the vector table (`.isr_vector`) plus the `Reset_Handler` that copies `.data` from flash to RAM, zeroes `.bss`, and calls `main`.
- **`linker.ld`** — the memory layout (flash `0x08000000`, RAM `0x20000000`) and the `_sidata`/`_sdata`/`_edata`/`_sbss`/`_ebss`/`_estack` symbols the startup code relies on.

## Build

Needs an `arm-none-eabi` GCC toolchain on your `PATH`:

```sh
./build.sh
```

or directly:

```sh
arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -nostdlib -ffreestanding -O0 -g \
    -T linker.ld startup.c main.c -o firmware.elf
arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
```

## Look inside

```sh
xxd firmware.bin | head -1          # first 8 bytes: initial SP + reset vector
arm-none-eabi-objdump -h firmware.elf   # section headers: note .data's VMA vs LMA
arm-none-eabi-objdump -d firmware.elf   # disassembly (Reset_Handler, vector table)
```
