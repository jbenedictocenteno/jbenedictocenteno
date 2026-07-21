# What are the stack and the heap?

<!--
WIP — placeholder, to be written later.

Angle: "the stack" and "the heap" get name-dropped constantly but are almost
always assumed known. This note should actually explain them from the ground
up, the same "no magic" way as the other notes.

Rough scope to flesh out:
- The stack: a region of RAM + the stack pointer (SP); stack frames; push/pop
  on function call/return; where locals and return addresses live; why it's
  fast; which direction it grows; what a stack overflow actually is.
- The heap: dynamic allocation (malloc/free / new/delete); the allocator's job;
  fragmentation; why it's slower; lifetime managed by hand.
- Stack vs heap side by side: lifetime, cost, size limits, who manages each.
- Where they sit in the memory map — ties in with the linker script and boot
  notes (see how_does_an_mcu_boot: _estack, heap and stack growing toward each
  other).
- Bare-metal angle: on an MCU there's often no real heap, and the stack size is
  fixed by the linker script.
-->
