cmake_minimum_required(VERSION 3.16)

# ---------------------------------------------------------------------------
# Toolchain - must be set before project()
# ---------------------------------------------------------------------------
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(RTOSApp C ASM)

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
set(SYS_SOURCES
    ${ROOT}/src/sys/startup.c
    ${ROOT}/src/sys/printf-stdarg.c
    ${ROOT}/src/sys/data_utils.c
)

set(DRIVER_SOURCES
    ${ROOT}/src/drivers/app_hal_sim.c
    ${ROOT}/src/drivers/drv_uart.c
)

set(SERVICE_SOURCES
    ${ROOT}/src/services/logger.c
)

set(APP_LOGIC_SOURCES
    ${ROOT}/src/app/task_uart.c
    ${ROOT}/src/app/task_sensors.c
)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
add_executable(RTOSApp
    ${ROOT}/src/main.c
    ${ROOT}/src/syscalls.c
    ${KERNEL_SOURCES}
    ${SYS_SOURCES}
    ${DRIVER_SOURCES}
    ${SERVICE_SOURCES}
    ${APP_LOGIC_SOURCES}
)

set_target_properties(RTOSApp PROPERTIES 
    OUTPUT_NAME "RTOSApp.elf"
)

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------
target_include_directories(RTOSApp PRIVATE
    ${ROOT}/include             # Master Menu (Forces syntax: #include "drivers/drv_uart.h")
    ${ROOT}/CMSIS               # Silicon definitions (core_cm3.h)
    ${KERNEL_DIR}/include       # FreeRTOS Core
    ${KERNEL_PORT_DIR}          # FreeRTOS Hardware Port (portmacro.h)
)

# ---------------------------------------------------------------------------
# Linker
# ---------------------------------------------------------------------------
set(LINKER_SCRIPT ${ROOT}/scripts/mps2_m3.ld)

target_link_options(RTOSApp PRIVATE
    ${CPU_FLAGS}
    -T${LINKER_SCRIPT}
    -Wl,-Map=${CMAKE_BINARY_DIR}/RTOSApp.map
    -Wl,--gc-sections
    -nostartfiles
    -specs=nano.specs
    # -specs=nosys.specs
)

set_target_properties(RTOSApp PROPERTIES
    LINK_DEPENDS ${LINKER_SCRIPT}
)

# ---------------------------------------------------------------------------
# Post-build: .elf -> .out + print sizes
# ---------------------------------------------------------------------------
# add_custom_command(TARGET RTOSApp POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#         $<TARGET_FILE:RTOSApp>
#         ${CMAKE_BINARY_DIR}/RTOSApp.out
#     COMMENT "Copying RTOSApp -> RTOSApp.out"
# )

find_program(ARM_SIZE arm-none-eabi-size)
if(ARM_SIZE)
    add_custom_command(TARGET RTOSApp POST_BUILD
        COMMAND ${ARM_SIZE} $<TARGET_FILE:RTOSApp>
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
        -kernel ${CMAKE_BINARY_DIR}/RTOSApp.elf
        -serial stdio
        -nographic
    DEPENDS RTOSApp.elf
    COMMENT "Launching QEMU..."
)