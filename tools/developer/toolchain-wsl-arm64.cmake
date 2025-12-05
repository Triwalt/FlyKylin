# Toolchain file for cross-compiling FlyKylin to ARM64 using WSL Ubuntu multi-arch
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Cross compilers (host-installed via apt)
set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)

# We are using the host fs with multi-arch support, so no separate sysroot root needed
# just point to the right architecture libs
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)

# Search behavior
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Explicitly search in multi-arch directories
set(CMAKE_IGNORE_PATH /usr/lib/x86_64-linux-gnu /usr/lib/i386-linux-gnu)
