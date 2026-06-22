#!/usr/bin/env bash
set -euo pipefail # Fail script instantly if any command fails or a variable is unset

BUILD_DIR="build"
BUILD_TYPE="Debug"

# Print colored headers
function info() { echo -e "\033[1;34m[BUILD]\033[0m $1"; }
function success() { echo -e "\033[1;32m[SUCCESS]\033[0m $1"; }

# ==============================================================================
# 1. THE "CLEAN" COMMAND
# ==============================================================================
if [[ "${1:-}" == "--clean" || "${1:-}" == "-c" || "${1:-}" == "clean" ]]; then
    info "Removing old build directory..."
    rm -rf "$BUILD_DIR"
    success "Project cleaned."
    exit 0
fi

# ==============================================================================
# 2. THE "MAKE" COMMAND
# ==============================================================================
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# If we just wiped the folder, or are building for the very first time:
if [ ! -f "Makefile" ]; then
    info "No Makefile detected. Generating build tree via CMake..."
    cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..
fi

info "Compiling across $(nproc) CPU cores..."
make -j"$(nproc)"

# (Updated the output name to match the .elf we declared in your CMakeLists!)
success "Build finished! Firmware located at: ./${BUILD_DIR}/freertos_mps2.elf"