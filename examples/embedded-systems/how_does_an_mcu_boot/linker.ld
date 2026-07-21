/* linker.ld — minimal layout for an STM32U575 (Cortex-M33). */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 192K   /* SRAM1 */
}

/* Top of RAM. The stack grows downward, so this is the initial SP —
   exactly the _estack the vector table put in entry 0. */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* Vector table, pinned to the very start of flash (0x08000000). */
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > FLASH

    /* Code and read-only data: executed/read in place, straight from flash. */
    .text :
    {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* Initialized data: lives in RAM at run time, but its initial image is
       stored in flash and copied across by Reset_Handler. */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    _sidata = LOADADDR(.data);

    /* Zero-initialized data: takes up RAM only, nothing stored in flash. */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM
}
