cmake_minimum_required(VERSION 3.15)

# 1. FORCE-LOCK TO BARE METAL ARM (Must happen BEFORE the project() call)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# >>> BYPASS THE LINKER SANITY CHECK <<<
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_ASM_COMPILER "arm-none-eabi-gcc")

project(freertos_qemu_sim C ASM)

set(CMAKE_C_STANDARD 99)

# 2. VALIDATE THE ENVIRONMENT VARIABLE
if(NOT DEFINED ENV{FREERTOS_DIR})
    message(FATAL_ERROR "\n[ERROR] Environment variable FREERTOS_DIR is missing!\nRun: export FREERTOS_DIR=/path/to/FreeRTOS-Kernel\n")
endif()

set(FREERTOS_DIR $ENV{FREERTOS_DIR})
message(STATUS "Using FreeRTOS Kernel from ENV: ${FREERTOS_DIR}")

# 3. SET CORTEX-M3 HARDWARE FLAGS
set(CPU_FLAGS "-mcpu=cortex-m3 -mthumb")
set(COMMON_FLAGS "${CPU_FLAGS} -g3 -O0 -Wall -ffunction-sections -fdata-sections")

set(CMAKE_C_FLAGS "${COMMON_FLAGS}" CACHE INTERNAL "")
set(CMAKE_ASM_FLAGS "${COMMON_FLAGS}" CACHE INTERNAL "")

# --specs=rdimon.specs wires the ARM C-library directly to QEMU's stdout
set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS} -Wl,--gc-sections --specs=rdimon.specs" CACHE INTERNAL "")

# 4. GATHER FREERTOS KERNEL FILES
set(FREERTOS_CORE_SOURCES
    "${FREERTOS_DIR}/tasks.c"
    "${FREERTOS_DIR}/queue.c"
    "${FREERTOS_DIR}/list.c"
    "${FREERTOS_DIR}/timers.c"
    "${FREERTOS_DIR}/portable/MemMang/heap_4.c"
    "${FREERTOS_DIR}/portable/GCC/ARM_CM3/port.c"
)

# 5. GATHER OUR APPLICATION FILES
set(APP_SOURCES
    src/main.c
    src/rtos_tasks.c
    src/app_hal_sim.c
    src/startup.c        # <--- SEE IMPORTANT NOTE BELOW
)

add_executable(freertos_mps2.elf ${FREERTOS_CORE_SOURCES} ${APP_SOURCES})

# 6. ATTACH THE LINKER SCRIPT
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/mps2_m3.ld")
target_link_options(freertos_mps2.elf PRIVATE "-T${LINKER_SCRIPT}")

target_include_directories(freertos_mps2.elf PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${FREERTOS_DIR}/include"
    "${FREERTOS_DIR}/portable/GCC/ARM_CM3"
)

# 7. BONUS: Automatically generate a .bin alongside the .elf
add_custom_command(TARGET freertos_mps2.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O binary freertos_mps2.elf freertos_mps2.bin
    COMMENT "Generated raw binary: freertos_mps2.bin"
)