#!/usr/bin/env sh
# Build the minimal STM32U575 (Cortex-M33) boot example.
# Requires an arm-none-eabi GCC toolchain on PATH.
set -e

arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -nostdlib -ffreestanding -O0 -g \
    -T linker.ld startup.c main.c -o firmware.elf

arm-none-eabi-objcopy -O binary firmware.elf firmware.bin

echo "Built firmware.elf and firmware.bin"
