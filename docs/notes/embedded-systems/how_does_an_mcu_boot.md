# How does an MCU gets to `main()`?

<!-- WIP — draft in progress, do not publish yet -->

In [How does a machine know C?](../programming/c-cpp/how_does_a_machine_know_c.md) we followed a `.c` file all the way down to the `.bin` that gets flashed, and watched the linker hand out final addresses: `.text`/`.rodata` in flash, `.data`/`.bss` in RAM. The linker only *decides where things live* — it doesn't actually put anything there. When the chip powers on, RAM is just noise: nobody has copied the initial values of our global variables into `.data`, and nobody has zeroed `.bss`. And there's a more basic question still unanswered: how does the CPU even start running code? A freshly reset CPU doesn't know what `main` is any more than it knew what C was.

## Introduction.

I'll target an **STM32U575** (Arm Cortex-M33). The exact part barely matters — the boot sequence is essentially the same across the whole Cortex-M family, and the same ideas carry over to most other cores too, give or take a few quirks and differences. But pinning it to a real chip lets us use real addresses and inspect real binaries. Two facts about this target, both read from the STM32U575 reference manual:

- **Flash** is mapped at `0x08000000`, and **SRAM** at `0x20000000`.
- The Cortex-M33 has **TrustZone**, but the part ships with it disabled (the `TZEN` option bit is `0`), so it boots in a single security state — a plain, classic Cortex-M boot.

"Reset" here just means the CPU is in its power-on state: registers cleared, nothing initialized, the program counter about to be loaded from a fixed place in flash. Everything below is the chip climbing out of that state and into our C program.

## What are the program counter and stack pointer?

Two CPU registers do most of the work in this note, so they're worth a quick refresher before we start leaning on them.

The **program counter (PC)** is a register inside the CPU that holds the address of the *next* instruction to run. The CPU's whole life is one tight loop: fetch the instruction PC points at, execute it, advance PC, repeat — forever. A jump, a branch, a function call: none of them are magic, they're just *writing a new address into PC*. So the question "how does the CPU start running our code?" is really the sharper question "**what address ends up in PC at reset?**".

The **stack pointer (SP)** holds the address of the top of the stack: the slice of RAM the CPU uses for return addresses, function arguments, and local variables. Almost nothing in C survives without it — even a plain function call pushes a return address onto the stack — so SP has to point at valid RAM *before* the first instruction of our program runs.

Here's the neat part, and the reason this refresher is up front: on a Cortex-M, these two registers are special enough that the **hardware loads both of them for us at reset, before a single instruction executes**. Where does it get the values? From the first two words of flash — the *vector table*.

## Example firmware.

To have something to work with, let's create a simple project:

```
prj/
|- main.c        the application (main() function)
|- startup.c     the vector table + Reset_Handler
|- linker.ld     the memory layout
```

Starting from the end — the application — because it's the part we actually care about, and because it quietly sets the requirements for everything else:

```c
#include <stdint.h>

volatile uint32_t counter = 7;   /* initialized  -> lands in .data */
volatile uint32_t flag;          /* zero-init    -> lands in .bss  */

int main(void)
{
    counter++;
    flag = 1;
    while (1) { }
}
```

These two globals are our testing probes into the booting process:

- `counter` has a non-zero initializer (`7`), so it lands in the `.data` section. That `7` lives in flash and has to be *copied* into the RAM address reserved for `counter` before `main` runs — otherwise `counter++` would just increment whatever garbage happened to be sitting at that direction at power-on.
- `flag` has no initializer, so it lands in `.bss`. The C standard promises it reads as `0` at program start — but nobody zeroes RAM for free, so *something* has to write that `0`.

That "something" is the startup code we're about to write — the whole point of this note is that those two guarantees don't happen by themselves. (`volatile` is just there so the compiler can't optimize the variables away; we want them to really exist in memory. And `<stdint.h>` is safe to include even in a bare-metal build — these *freestanding* headers ship with the compiler itself (the `arm-none-eabi-gcc` toolchain from [How does a machine know C?](../programming/c-cpp/how_does_a_machine_know_c.md)), so no C library is required.)

We'll compile all three files with a single bare-metal invocation of `arm-none-eabi-gcc` and flatten the result into an image — but that command only makes sense once `startup.c` and `linker.ld` exist. So let's first take a look at `startup.c`:

## `startup.c`.

`startup.c` has two jobs, and together they *are* the boot process:

1. Provide the **vector table**, so the CPU knows where to begin.
2. Provide the **`Reset_Handler`** — the very first code that runs. It makes good on the `.data`/`.bss` guarantees from above and then calls `main`.

Let's take them in that order.

### The vector table.

The vector table is nothing but an **array of addresses** sitting at the very start of flash. On reset the CPU reads the first two entries — nothing has to "install" it, the linker simply places this array first, at `0x08000000`. Here's the top of `startup.c`:

```c
#include <stdint.h>

/* Defined by the linker script: the top of RAM, which we'll use
   as the initial stack pointer. */
extern uint32_t _estack;

/* Defined further down in this same file. */
void Reset_Handler(void);

/* Each entry is just the address of something. Declaring them as
   function pointers makes the compiler bake in the Thumb bit for us. */
typedef void (*vector_t)(void);

__attribute__((section(".isr_vector"), used))
const vector_t vector_table[] = {
    (vector_t)&_estack,   /* 0x00  initial stack pointer (loaded into SP) */
    Reset_Handler,        /* 0x04  reset vector        (loaded into PC)   */
    /* NMI, HardFault, ... would continue here — omitted until we need them */
};
```

Four things worth unpacking:

- **Entry 0 — the initial stack pointer.** Remember the SP refresher: it must point at valid RAM *before* the first instruction. `_estack` is the top of RAM (`0x20000000` + the RAM size), a value the linker script hands us. On Cortex-M the stack grows *downward*, so we start it at the highest address and let it grow down from there. The hardware copies this word straight into SP.
- **Entry 1 — the reset vector.** The address of `Reset_Handler`. The hardware copies this word into PC, so execution begins there. This one entry is the entire answer to "how does the CPU start running *our* code?" from the intro.
- **`.isr_vector` and `used`.** We tag the array into its own section so the linker script can force it to the base of flash (we'll write that placement into `linker.ld`). The `used` attribute stops the compiler from garbage-collecting the array — nothing ever *calls* `vector_table`, so without it the optimizer could legitimately drop the whole thing, and the chip would boot on garbage.
- **The Thumb bit, for free.** Cortex-M only ever runs *Thumb* instructions, and it marks that by keeping bit 0 of every function address set to `1`. Because we listed `Reset_Handler` as a function pointer, that odd address is stored for us automatically. Hand-write the table in assembly with a plain even address and the CPU faults on the spot: it reads bit 0 as `0`, tries to switch to ARM state (which Cortex-M doesn't have), and locks up before running a single instruction. (`_estack` is a data address, not a function, so it's the one entry we cast — the hardware just loads it into SP as a raw number, so its type doesn't matter to the silicon.)

We can't *see* these two hexadecimal words yet (PC & SP): the value of `_estack` isn't pinned down until the linker script exists and we actually build.

Next, the code the reset vector points at: `Reset_Handler`.

### `Reset_Handler`.

The reset vector points here, so this is the first instruction the CPU runs. Its job is to build the C environment `main` expects — the `.data` and `.bss` guarantees we set up as probes — and then hand over to main().

```c
/* All provided by the linker script (linker.ld) — see the next section.
   These are addresses, not variables: we only ever take their address. */
extern uint32_t _sidata;   /* .data's initial values, stored in flash (LMA) */
extern uint32_t _sdata;    /* .data start in RAM (VMA)                       */
extern uint32_t _edata;    /* .data end   in RAM                             */
extern uint32_t _sbss;     /* .bss start  in RAM                             */
extern uint32_t _ebss;     /* .bss end    in RAM                             */

int main(void);

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
```

That's the whole thing — the same work a vendor's `startup_stm32u5xx.s` does, just written out in a few lines of C. Point by point:

- **Those `extern` symbols are addresses, not data.**. `_sdata` and friends are defined by the *linker*, not by any C code — the "contents" of `_sdata` are meaningless. That's why we always write `&_sdata`, never `_sdata`: it's just how the linker tells our C code "Hey here's where my `.data` starts / ends".
- **Step 1 — why  does `.data` needs copying at all (LMA vs VMA)??.** At runtime our initialized globals live in RAM (`_sdata`..`_edata`). But RAM is volatile — it's garbage at power-on — so their *initial* values have to be kept somewhere non-volatile and copied over. The linker stashes that copy in flash, right after the code, and calls its start `_sidata` (the **L**oad address LMA), while the variables' run-time home is `_sdata` (the **V**irtual, i.e. run-time, address VMA). Step 1 is just `memcpy(_sdata, _sidata, _edata - _sdata)` spelled out by hand. This is the missing half of the previous note: the linker *decided* `.data` lives in RAM — here's the code that actually *puts* it there.
- **Step 2 — why `.bss` is separate.** `.bss` is all zeros, so there's nothing to store in flash — a block of zeros in the binary would just be wasted space. The linker only records the range, and we zero it here. That's why an all-zero global costs nothing in flash while a `= 7` global costs four bytes :exploding_head:.
- **The subtle part: this code runs *before* the world it's building exists.** When `Reset_Handler` starts, `.data` still holds garbage and `.bss` isn't zeroed yet — so it must not read or write any global that lives there, or it'd be trusting values it's in the middle of setting up. Notice it only ever touches linker symbols (constants) and *local* variables. Those locals live on the **stack**, which already works because entry 0 of the vector table set `SP` for us before this function was even entered. The stack is the one piece of the C runtime that's ready for free.
- **What we're skipping.** A production startup does two more things in before `main`: `SystemInit()` (clock and FPU setup) and `__libc_init_array()` (C++ static constructors and anything in `.init_array`). We don't need clocks to run this example, and we aren't linking a C library, so we go straight to `main`.

Every symbol `Reset_Handler` leans on — `_sidata`, `_sdata`, `_edata`, `_sbss`, `_ebss`, plus `_estack` from the vector table — is handed to us by the **linker script**.

## `linker.ld`.

The startup code kept deferring to "symbols the linker script hands us". Here's the script that defines them — and it's short. A linker script does just two things: it declares the chip's memory regions, and it says which section goes where. (For the anatomy of sections and what the linker does with them, see the linker step in [How does a machine know C?](../programming/c-cpp/how_does_a_machine_know_c.md).)

```ld
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
```

Reading it top to bottom:

- **`MEMORY`** is where the two addresses from the intro finally become concrete: flash at `0x08000000`, RAM at `0x20000000`. The `(rx)`/`(rwx)` flags are just descriptive attributes. (I kept `RAM` at SRAM1's 192 KB; the U575 actually has several SRAM banks, but this is plenty — the exact size only matters for where `_estack` lands.)
- **`_estack = ORIGIN(RAM) + LENGTH(RAM)`** is the top of RAM, and the circle closes here: the linker computes it, the vector table stores it in entry 0, and the hardware loads it into `SP` at reset. The stack grows *down* from this address.
- **`ENTRY(Reset_Handler)`** names the program's entry point in the ELF header. On real silicon it's redundant — the chip boots through the vector table, not the ELF entry point — but debuggers and simulators use it to know where execution begins, and it keeps the linker from warning that it *"cannot find entry symbol"*.
- **`.isr_vector` first, with `KEEP`.** Placing it first in `> FLASH` pins it to `0x08000000`, exactly where the CPU looks on reset. `KEEP` stops `--gc-sections` from discarding it — the same worry as the `used` attribute back in the vector table, belt-and-suspenders since nothing ever references it.
- **`.text`/`.rodata` → flash.** Code and constants are read in place from non-volatile flash; they never need copying to RAM.
- **`.data` → `> RAM AT> FLASH` — the entire LMA/VMA trick in one line.** `> RAM` sets the *run-time* addresses (`_sdata`..`_edata`, in RAM — the VMA). `AT> FLASH` sets the *load* address (where the bytes actually sit in the image — in flash, right after `.text` — the LMA). Then `_sidata = LOADADDR(.data)` captures that flash address. Those three symbols are precisely what `Reset_Handler`'s copy loop reads: the split we coded by hand is *declared* right here.
- **`.bss (NOLOAD)`.** `NOLOAD` tells the linker to reserve the RAM range but store nothing in the image — the literal meaning of "a block of zeros costs nothing in flash". `_sbss`/`_ebss` bound the range `Reset_Handler` zeroes; `*(COMMON)` sweeps up any tentative definitions.
- **The `ALIGN(4)`s** keep every boundary word-aligned, so `Reset_Handler`'s word-at-a-time copy and zero loops never trip over a misaligned start or end.

And that's all three files. Let's go to the cool part: **build it, and look inside the binary**.

## Building it, and looking inside.

The three files are in the repo (see the box below). Building them is one compiler call plus one `objcopy` — no build system in sight:

```
arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -nostdlib -ffreestanding -O0 -g \
    -T linker.ld startup.c main.c -o firmware.elf
arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
```

`-nostdlib` tells GCC not to pull in the C runtime or standard library — we supply our own startup, which is the whole point — `-ffreestanding` says there's no hosted OS underneath, and `-T linker.ld` hands over our memory map. `objcopy` then flattens the ELF into the raw image a flasher would write to `0x08000000`: a whole 136 bytes.

Lets take a look ath the firmware.bin generated file.**The two words the hardware reads at reset** are the first eight bytes of the flat image (`od -A x -t x4 -N 8 firmware.bin`, shown as 32-bit words):

```
000000 20030000 08000009
000008
```

Exactly what we drew:

- `0x20030000` — the initial stack pointer: `0x20000000` (base of RAM) + `0x30000` (192 KB) = the top of RAM, straight from `_estack`. The hardware loads it into `SP`.
- `0x08000009` — the reset vector. `Reset_Handler` lives at `0x08000008` (right after the 8-byte table) and the low bit is set — `…8 | 1 = …9` — the **Thumb bit**, there for free because we listed a function pointer. The hardware loads it into `PC` and execution begins. The entire "how does the CPU start running our code?" question, answered in eight bytes.

**`.data` really does live at two addresses.** The section headers (`arm-none-eabi-objdump -h firmware.elf`, debug sections and per-section flag lines trimmed):

```
Idx Name          Size      VMA       LMA       File off  Algn
  0 .isr_vector   00000008  08000000  08000000  00001000  2**2
  1 .text         0000007c  08000008  08000008  00001008  2**2
  2 .data         00000004  20000000  08000084  00002000  2**2
  3 .bss          00000004  20000004  08000088  00002004  2**2
```

Look at `.data`: its **VMA is `0x20000000`** (its run-time home in RAM) while its **LMA is `0x08000084`** (where its initial bytes are stored, in flash, right after `.text`). Same four bytes, two addresses — the entire reason `Reset_Handler` needs a copy loop. In the full listing `.bss` carries only the `ALLOC` flag, no `CONTENTS`/`LOAD`: it takes up RAM but nothing in the file, exactly what `NOLOAD` promised.

**Every symbol the startup leaned on, now a concrete address** (`arm-none-eabi-nm -n firmware.elf`, sorted by address):

```
08000000 R vector_table
08000008 T Reset_Handler
08000064 T main
08000084 A _sidata
20000000 D _sdata
20000000 D counter
20000004 D _edata
20000004 B _sbss
20000004 B flag
20000008 B _ebss
20030000 R _estack
```

Read top to bottom, it's the whole note on one screen. `vector_table` sits at `0x08000000`, so the CPU finds it on reset. `counter` shares an address with `_sdata` (`0x20000000`) — it *is* the first thing in `.data` — and its initial `7` waits at `_sidata` (`0x08000084`) out in flash for the copy loop to fetch. `flag` sits at `_sbss` (`0x20000004`), inside the range the zero loop clears. `_estack` is `0x20030000` — the very word we read out of the first eight bytes. Nothing is hidden: every name the hand-written startup referenced is just an address the linker filled in.

So by the time `Reset_Handler` falls into `main`, `counter` holds `7` and `flag` holds `0` — not by magic, but because the handful of lines in `startup.c` and the memory map in `linker.ld` put them there.

!!! note "Grab the code"
    The complete, buildable example is in the repo at
    [`examples/embedded-systems/how_does_an_mcu_boot/`](https://github.com/jbenedictocenteno/jbenedictocenteno/tree/main/examples/embedded-systems/how_does_an_mcu_boot):
    `main.c`, `startup.c`, `linker.ld`, and a `build.sh` with the commands above.
