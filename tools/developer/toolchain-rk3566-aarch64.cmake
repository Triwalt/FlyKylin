# Toolchain file for cross-compiling FlyKylin to RK3566 (Linux aarch64)
# using Ubuntu 20.04 multi-arch packages (Qt5 5.12.8, Protobuf, etc.).

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Cross compilers (host-installed)
set(CMAKE_C_COMPILER   /usr/bin/aarch64-linux-gnu-gcc-10)
set(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++-10)
set(CMAKE_ASM_COMPILER /usr/bin/aarch64-linux-gnu-gcc-10)

# Sysroot copied from RK3566 board (must be prepared manually under WSL):
#   /opt/rk3566/sysroot
set(CMAKE_SYSROOT "/opt/rk3566/sysroot")

# Prefer target (arm64) libraries/includes from the sysroot over host ones
set(CMAKE_FIND_ROOT_PATH
    "${CMAKE_SYSROOT}"
)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# For CMake packages (like Qt5), search both host and sysroot so that we can
# use host-provided Qt5 CMake config while still linking against target libs.
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
