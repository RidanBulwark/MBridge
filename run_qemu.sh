#!/usr/bin/env bash
set -euo pipefail

# This must point to the cross-compiled ARM .elf, NOT the native POSIX binary
ELF_PATH="./build/RTOSApp.elf"

if [ ! -f "$ELF_PATH" ]; then
    echo -e "\033[1;31m[ERROR]\033[0m Firmware binary '$ELF_PATH' not found!"
    echo "Please cross-compile for ARM Cortex-M3 first."
    exit 1
fi

echo -e "\033[1;33m[BASH]\033[0m Booting virtual ARM Cortex-M3 (MPS2-AN385)..."
echo "------------------------------------------------------------------"


qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -kernel $ELF_PATH\
                 -monitor none -nographic -serial stdio \
                -semihosting \
                -semihosting-config enable=on,target=native

# ANATOMY OF THESE FLAGS (For your knowledge):
# -machine mps2-an385 : Tells QEMU the exact Memory Map of the motherboard to fake.
# -nographic          : Disables the pop-up LCD window (run purely inside this terminal).
# -serial stdio       : Takes the simulated microcontroller's UART0 TX pin and pipes it into your bash window.
# -monitor null       : Hides the annoying (qemu) debug prompt.
# -semihosting        : Allows the ARM chip to use your Host PC's standard C library for things like 'printf'.