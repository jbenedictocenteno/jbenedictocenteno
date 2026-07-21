#include <stdint.h>

/* ---- symbols provided by the linker script (linker.ld) ----
   These are addresses, not variables: we only ever take their address. */
extern uint32_t _estack;   /* top of RAM: initial stack pointer      */
extern uint32_t _sidata;   /* .data's initial values, in flash (LMA) */
extern uint32_t _sdata;    /* .data start in RAM (VMA)               */
extern uint32_t _edata;    /* .data end   in RAM                     */
extern uint32_t _sbss;     /* .bss start  in RAM                     */
extern uint32_t _ebss;     /* .bss end    in RAM                     */

int  main(void);
void Reset_Handler(void);

/* ---- vector table ----
   Declaring the entries as function pointers makes the compiler bake in
   the Thumb bit for us. Placed in .isr_vector so the linker can pin it to
   the very start of flash. */
typedef void (*vector_t)(void);

__attribute__((section(".isr_vector"), used))
const vector_t vector_table[] = {
    (vector_t)&_estack,   /* 0x00  initial stack pointer (loaded into SP) */
    Reset_Handler,        /* 0x04  reset vector        (loaded into PC)   */
    /* NMI, HardFault, ... would continue here — omitted until we need them */
};

/* ---- reset handler ----
   The first code that runs. Builds the C environment main() expects, then
   hands over. It runs BEFORE .data/.bss are initialized, so it must only
   touch linker symbols and local (stack) variables. */
void Reset_Handler(void)
{
    /* 1. Copy .data from flash into RAM. */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* 2. Zero-fill .bss. */
    for (uint32_t *p = &_sbss; p < &_ebss; p++) {
        *p = 0;
    }

    /* 3. Hand control to the application. */
    main();

    /* 4. main() shouldn't return on bare metal; if it does, spin forever. */
    while (1) { }
}
