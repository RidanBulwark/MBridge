cmake_minimum_required(VERSION 3.16)

# ---------------------------------------------------------------------------
# Toolchain - must be set before project()
# ---------------------------------------------------------------------------
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(RTOSDemo C ASM)

# ---------------------------------------------------------------------------
# Paths - all relative to this CMakeLists.txt
# ---------------------------------------------------------------------------
set(ROOT            ${CMAKE_CURRENT_SOURCE_DIR})
set(KERNEL_DIR      ${ROOT}/freertos)
set(KERNEL_PORT_DIR ${KERNEL_DIR}/portable/GCC/ARM_CM3)

# ---------------------------------------------------------------------------
# Compiler flags
# ---------------------------------------------------------------------------
set(CPU_FLAGS -mthumb -mcpu=cortex-m3 -ffreestanding)

add_compile_options(
    ${CPU_FLAGS}
    -g3
    -Os
    -ffunction-sections
    -fdata-sections
    -Wall
    -Wextra
    -Wshadow
    -Wno-unused-value
)

# ---------------------------------------------------------------------------
# FreeRTOS kernel sources
# ---------------------------------------------------------------------------
set(KERNEL_SOURCES
    ${KERNEL_DIR}/tasks.c
    ${KERNEL_DIR}/list.c
    ${KERNEL_DIR}/queue.c
    ${KERNEL_DIR}/timers.c
    ${KERNEL_DIR}/event_groups.c
    ${KERNEL_DIR}/stream_buffer.c
    ${KERNEL_DIR}/portable/MemMang/heap_4.c
    ${KERNEL_PORT_DIR}/port.c
)

# ---------------------------------------------------------------------------
# Application sources
# ---------------------------------------------------------------------------
set(APP_SOURCES
    ${ROOT}/src/main.c
    ${ROOT}/src/rtos_tasks.c
    ${ROOT}/src/app_hal_sim.c
    ${ROOT}/src/logger.c
    ${ROOT}/src/printf-stdarg.c
    ${ROOT}/src/startup.c
)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
add_executable(rtos_app.elf
    ${KERNEL_SOURCES}
    ${APP_SOURCES}
)

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------
target_include_directories(rtos_app.elf PRIVATE
    ${ROOT}/include             # FreeRTOSConfig.h, app_hal.h, logger.h, data_types.h
    ${ROOT}/CMSIS               # core_cm3.h, SMM_MPS2.h etc.
    ${KERNEL_DIR}/include       # FreeRTOS.h, task.h, queue.h, semphr.h etc.
    ${KERNEL_PORT_DIR}          # portmacro.h
)

# ---------------------------------------------------------------------------
# Linker
# ---------------------------------------------------------------------------
set(LINKER_SCRIPT ${ROOT}/scripts/mps2_m3.ld)

target_link_options(rtos_app.elf PRIVATE
    ${CPU_FLAGS}
    -T${LINKER_SCRIPT}
    -Wl,-Map=${CMAKE_BINARY_DIR}/RTOSDemo.map
    -Wl,--gc-sections
    -nostartfiles
    -specs=nano.specs
    -specs=nosys.specs
)

set_target_properties(rtos_app.elf PROPERTIES
    LINK_DEPENDS ${LINKER_SCRIPT}
)

# ---------------------------------------------------------------------------
# Post-build: .elf -> .out + print sizes
# ---------------------------------------------------------------------------
add_custom_command(TARGET rtos_app.elf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:rtos_app.elf>
        ${CMAKE_BINARY_DIR}/RTOSDemo.out
    COMMENT "Copying rtos_app.elf -> RTOSDemo.out"
)

find_program(ARM_SIZE arm-none-eabi-size)
if(ARM_SIZE)
    add_custom_command(TARGET rtos_app.elf POST_BUILD
        COMMAND ${ARM_SIZE} $<TARGET_FILE:rtos_app.elf>
        COMMENT "Section sizes:"
    )
endif()

# ---------------------------------------------------------------------------
# 'cmake --build . --target run'  or  'make run'
# ---------------------------------------------------------------------------
add_custom_target(run
    COMMAND qemu-system-arm
        -machine mps2-an385
        -monitor null
        -semihosting
        -semihosting-config enable=on,target=native
        -kernel ${CMAKE_BINARY_DIR}/rtos_app.elf
        -serial stdio
        -nographic
    DEPENDS rtos_app.elf
    COMMENT "Launching QEMU..."
)