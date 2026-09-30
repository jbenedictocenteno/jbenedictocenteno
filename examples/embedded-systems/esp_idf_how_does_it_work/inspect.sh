#!/usr/bin/env sh
# Every command the note "How does ESP-IDF work?" shows, in the order it shows
# them. Run after `idf.py set-target esp32p4 && idf.py build`, with the IDF
# environment active (`. $IDF_PATH/export.sh`).
#
# Output goes to stdout and to dumps/, which is gitignored.
set -e

ELF=build/idf_tour.elf
BIN=build/idf_tour.bin
OBJDUMP=riscv32-esp-elf-objdump
NM=riscv32-esp-elf-nm

if [ ! -f "$ELF" ]; then
    echo "No $ELF — run 'idf.py build' first." >&2
    exit 1
fi

mkdir -p dumps

section() {
    echo
    echo "==============================================================="
    echo "== $1"
    echo "==============================================================="
}

section "1. Kconfig round trip: our option, from sdkconfig to a #define"
grep -n "MY_LED" sdkconfig
grep -n "MY_LED" build/config/sdkconfig.h
grep -n "MY_LED" build/config/sdkconfig.cmake

section "2. Which components ended up in the build"
python - <<'PY'
import json
d = json.load(open("build/project_description.json"))
print("target        :", d["target"])
print("toolchain     :", d.get("monitor_toolprefix", "?"))
print("components    :", len(d["build_component_info"]))
for name in ("main", "my_led", "esp_driver_gpio", "esp_hal_gpio", "soc"):
    info = d["build_component_info"].get(name)
    if info:
        print(f"  {name:16} reqs={info['reqs']} priv_reqs={info['priv_reqs']}")
PY

section "3. Section headers: where code and data actually live"
$OBJDUMP -h "$ELF" | tee dumps/sections.txt

section "4. Our own symbols, sorted by address"
$NM -n "$ELF" | grep -E " (app_main|app_blink_isr_safe|my_led_toggle_fast|my_led_set|my_led_init|counter|flag)$" \
    | tee dumps/symbols.txt

section "5. The generated linker script (nobody wrote this by hand)"
# Two lines, from two different output sections: one sends my_led_toggle_fast
# to IRAM, the other leaves the rest of my_led.c in flash. ldgen wrote both
# from our four-line linker.lf. (Long catch-all lines are filtered out.)
grep -n "libmy_led" build/esp-idf/esp_system/ld/sections.ld | awk 'length($0) < 200'

section "6. First bytes of the app image: the ESP image header"
od -A d -t x1 -N 24 "$BIN"

section "7. The same header, decoded by esptool"
python -m esptool --chip esp32p4 image-info "$BIN" | tee dumps/image-info.txt

section "8. The partition table, decoded back from its binary form"
python "$IDF_PATH/components/partition_table/gen_esp32part.py" \
    build/partition_table/partition-table.bin | tee dumps/partitions.txt

section "9. What gets written to flash, and where"
cat build/flasher_args.json

echo
echo "Dumps written to dumps/"
