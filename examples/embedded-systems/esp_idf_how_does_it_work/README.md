# ESP-IDF tour project

The project taken apart in the note
[How does ESP-IDF work?](../../../docs/notes/embedded-systems/esp_idf_how_does_it_work.md).

It is deliberately small, but it has one of everything the note talks about:

- **`CMakeLists.txt`** — the four-line boilerplate, and why the order of the lines matters.
- **`main/main.c`** — `app_main()`, with a `.data` probe (`counter = 7`) and a `.bss` probe (`flag`), the same two used in the bare-metal STM32 note. It prints them, plus the run-time addresses of four symbols that live in three different memory regions.
- **`components/my_led/`** — a hand-written component with its own `Kconfig` (adds `CONFIG_MY_LED_GPIO` to `menuconfig`) and its own `linker.lf` (moves one single function into IRAM).
- **`partitions.csv`** — a custom partition table, so you can compare the CSV against the binary the build generates.
- **`sdkconfig.defaults`** — the settings worth committing, including the two that lower the required chip revision.

## Build

Needs ESP-IDF v6.x. Written against v6.0.2 on an ESP32-P4.

```sh
. $IDF_PATH/export.sh
idf.py set-target esp32p4
idf.py build
```

To run it on a board:

```sh
idf.py -p /dev/cu.usbmodemXXXX flash monitor
```

`sdkconfig.defaults` targets a 16 MB flash and an ESP32-P4 of silicon revision v1.x. If your board differs, adjust `CONFIG_ESPTOOLPY_FLASHSIZE_16MB` and drop the two `CONFIG_ESP32P4_*REV*` lines. The LED pin defaults to GPIO 27 and is changeable under `menuconfig` → `my_led`.

## Look inside

```sh
./inspect.sh
```

Runs, in the order the note shows them:

1. **Kconfig round trip** — `CONFIG_MY_LED_GPIO` in `sdkconfig`, in the generated `sdkconfig.h`, and in `sdkconfig.cmake`.
2. **Component graph** — the target, the toolchain, and the `REQUIRES`/`PRIV_REQUIRES` chain from `main` down to `soc`, read out of `build/project_description.json`.
3. **Section headers** — `objdump -h`, showing the flash window at `0x400xxxxx` and internal RAM at `0x4ffxxxxx`.
4. **Symbols** — `nm -n`, showing which of our functions ended up in flash and which in IRAM.
5. **The generated linker script** — the two lines ldgen wrote from `components/my_led/linker.lf`, one per output section.
6. **The image header** — the first 24 bytes of the `.bin`, which decode field by field against `esp_image_header_t`.
7. **The same header via esptool** — `esptool image-info`, to check the hand decode.
8. **The partition table** — the binary blob at `0x8000` decoded back to CSV.
9. **`flasher_args.json`** — what gets written to flash and at which offsets.

Dumps are also written to `dumps/` (gitignored).
