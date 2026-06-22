# FreeRTOS ARM Cortex-M3 (QEMU) Starter Template

A zero-hardware, bare-metal C boilerplate for developing, building, and running **FreeRTOS** on an emulated **ARM Cortex-M3** (MPS2 AN385) via QEMU. 

This project implements a decoupled **Hardware Abstraction Layer (HAL)** pattern, allowing you to write application tasks against generic interfaces today, and swap out the underlying hardware drivers tomorrow.

---

## 📁 Project Architecture

```text
├── CMakeLists.txt        # Cross-compilation build instructions
├── include/
│   ├── FreeRTOSConfig.h  # FreeRTOS kernel parameters (tick rate, stack sizes, memory allocation)
│   ├── app_hal.h         # Hardware Abstraction Layer interface (Prototypes)
│   └── data_types.h      # Global project typedefs and structs
├── make.sh               # One-click build wrapper
├── run_qemu.sh           # Emulation launch script 
├── scripts/
│   └── mps2_m3.ld        # GNU Linker script (Memory map: Flash @ 0x0, RAM @ 0x20000000)
└── src/
    ├── app_hal_sim.c     # Simulated HAL implementation (maps hardware calls to QEMU semi-hosting)
    ├── main.c            # Entry point: Hardware init, Task creation, and Scheduler launch
    ├── rtos_tasks.c      # FreeRTOS thread definitions
    └── startup.c         # Bare-metal ARM bootloader (Vector table, Reset_Handler, .bss zeroing)