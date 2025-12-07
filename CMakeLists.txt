cmake_minimum_required(VERSION 3.20)

project(FlyKylin 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "AI-Powered P2P LAN Communication Tool"
)

# C++20标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 自动处理Qt的MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 平台检测
if(WIN32)
    set(PLATFORM_NAME "Windows")
    add_compile_definitions(Q_OS_WIN)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_NAME "Linux")
    add_compile_definitions(Q_OS_LINUX)

    # 根据目标体系结构和可选开关检测 RK3566 (ARM64)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(IS_RK3566 ON)
    endif()

    option(FLYKYLIN_FORCE_RK3566 "Force building for RK3566 target (e.g. cross-compilation)" OFF)
    if(FLYKYLIN_FORCE_RK3566)
        set(IS_RK3566 ON)
    endif()

    if(IS_RK3566)
        add_compile_definitions(RK3566_PLATFORM)
        message(STATUS "Building for RK3566 ARM64 platform")
    endif()
endif()

message(STATUS "Building for ${PLATFORM_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

# 查找依赖
# Qt 由本地环境或CI安装提供，这里默认优先查找 Qt6，不存在则回退到 Qt5。
# 在需要显式使用 Qt5（例如交叉编译或专门的 Qt5 环境）时，可以开启 FLYKYLIN_FORCE_QT5。

option(FLYKYLIN_FORCE_QT5 "Prefer Qt5 over Qt6 when both are available" OFF)

set(_FLYKYLIN_QT_COMPONENTS
    Core
    Gui
    Widgets
    Network
    Sql
    Test
    Qml
    Quick
    QuickControls2
)

# QtConcurrent 在 RK3566 rootfs 中不存在，因此在该平台上不要依赖 QtConcurrent 模块。
if(NOT IS_RK3566)
    list(APPEND _FLYKYLIN_QT_COMPONENTS Concurrent)
endif()

if(FLYKYLIN_FORCE_QT5)
    find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS ${_FLYKYLIN_QT_COMPONENTS})
else()
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ${_FLYKYLIN_QT_COMPONENTS})
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${_FLYKYLIN_QT_COMPONENTS})

# When cross-compiling for RK3566, we may only have Qt5 runtime libraries in
# the target sysroot but use host Qt5 development files (headers/CMake
# config). In that case, make sure the Qt imported targets link against the
# aarch64 libraries from the sysroot instead of host x86_64 libraries.
if(IS_RK3566 AND QT_VERSION_MAJOR EQUAL 5)
    set(_QT5_AARCH64_LIB_DIR "${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu")
    foreach(_qtmod Core Gui Widgets Network Sql Concurrent Test Qml Quick QuickControls2)
        if(TARGET Qt5::${_qtmod})
            # Explicitly probe common libQt5*.so paths in the RK3566 sysroot
            set(_qt_candidates
                "${_QT5_AARCH64_LIB_DIR}/libQt5${_qtmod}.so"
                "${_QT5_AARCH64_LIB_DIR}/libQt5${_qtmod}.so.5"
                "${_QT5_AARCH64_LIB_DIR}/libQt5${_qtmod}.so.5.12"
                "${_QT5_AARCH64_LIB_DIR}/libQt5${_qtmod}.so.5.12.8"
            )

            set(Qt5${_qtmod}_AARCH64_LIB "")
            foreach(_cand IN LISTS _qt_candidates)
                if(EXISTS "${_cand}")
                    set(Qt5${_qtmod}_AARCH64_LIB "${_cand}")
                    break()
                endif()
            endforeach()

            if(Qt5${_qtmod}_AARCH64_LIB)
                set_target_properties(Qt5::${_qtmod} PROPERTIES
                    IMPORTED_LOCATION "${Qt5${_qtmod}_AARCH64_LIB}"
                    IMPORTED_LOCATION_RELEASE "${Qt5${_qtmod}_AARCH64_LIB}")
                message(STATUS "[RK3566] Qt5::${_qtmod} -> ${Qt5${_qtmod}_AARCH64_LIB}")
            else()
                get_target_property(_qt_orig_loc Qt5::${_qtmod} IMPORTED_LOCATION_RELEASE)
                message(WARNING "[RK3566] Failed to find aarch64 Qt5${_qtmod} in ${_QT5_AARCH64_LIB_DIR}, keeping host lib: ${_qt_orig_loc}")
            endif()
        endif()
    endforeach()
endif()

# Protobuf + Abseil + utf8_range are provided by vcpkg on Windows/CI; prefer
# the vcpkg package configs there. On Linux/WSL, fall back to system packages
# if CONFIG packages are not available.
if(VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
    set(Protobuf_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/protobuf")
    set(absl_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/absl")
    set(utf8_range_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/utf8_range")
endif()

if(WIN32)
    find_package(Protobuf CONFIG REQUIRED)
    find_package(absl CONFIG REQUIRED)
    find_package(utf8_range CONFIG REQUIRED)
else()
    # 在非 Windows 平台上始终使用 CMake 的模块模式，绑定到系统 Protobuf
    # (protobuf-compiler/libprotobuf-dev)，避免误用 Windows/vcpkg/Anaconda 的
    # Protobuf CONFIG 包和 protoc.exe。
    #
    # 交叉编译到 RK3566 时，使用主机上的 Protobuf 头文件，避免 FindProtobuf
    # 去读取 sysroot 中不完整或版本不匹配的头文件而导致配置失败。
    if(IS_RK3566)
        set(Protobuf_INCLUDE_DIR "/usr/include" CACHE PATH "Host Protobuf include dir for RK3566 cross-build" FORCE)
    endif()
    find_package(Protobuf REQUIRED MODULE)
endif()

# 在 RK3566 交叉编译场景下，避免 Protobuf_LIBRARIES 中包含指向 sysroot 的绝对
# libprotobuf.so 路径（会被 Ninja 视为显式依赖，从而在文件缺失时中断）；统一
# 使用按名称链接的形式，由交叉链接器通过搜索路径解析。
if(IS_RK3566)
    set(Protobuf_LIBRARIES protobuf pthread CACHE STRING "Protobuf libraries for RK3566 cross-build" FORCE)
endif()

# ONNX Runtime
# 注意：Python环境中有onnxruntime 1.23.2，C++ API需要单独下载
# 从 https://github.com/microsoft/onnxruntime/releases 下载预编译库
set(ONNXRUNTIME_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/onnxruntime" CACHE PATH "ONNX Runtime root directory")
set(ONNXRUNTIME_FOUND FALSE)

if(UNIX AND NOT APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
        set(_FLYKYLIN_ONNXRUNTIME_CANDIDATE "${CMAKE_SOURCE_DIR}/3rdparty/onnxruntime-linux-x64-1.23.2")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(_FLYKYLIN_ONNXRUNTIME_CANDIDATE "${CMAKE_SOURCE_DIR}/3rdparty/onnxruntime-linux-aarch64-1.23.2")
    endif()

    if(DEFINED _FLYKYLIN_ONNXRUNTIME_CANDIDATE AND EXISTS "${_FLYKYLIN_ONNXRUNTIME_CANDIDATE}/include/onnxruntime_cxx_api.h")
        if(ONNXRUNTIME_ROOT STREQUAL "${CMAKE_SOURCE_DIR}/3rdparty/onnxruntime")
            set(ONNXRUNTIME_ROOT "${_FLYKYLIN_ONNXRUNTIME_CANDIDATE}" CACHE PATH "ONNX Runtime root directory" FORCE)
        endif()
    endif()
endif()

if(WIN32)
    if(EXISTS "${ONNXRUNTIME_ROOT}/include/onnxruntime_cxx_api.h" AND EXISTS "${ONNXRUNTIME_ROOT}/lib/onnxruntime.lib")
        message(STATUS "Found ONNX Runtime: ${ONNXRUNTIME_ROOT}")
        set(ONNXRUNTIME_FOUND TRUE)
        link_directories(${ONNXRUNTIME_ROOT}/lib)
    else()
        message(WARNING "ONNX Runtime C++ headers or libs not found in ${ONNXRUNTIME_ROOT}")
        message(WARNING "Please download from: https://github.com/microsoft/onnxruntime/releases")
        message(WARNING "Extract to: ${CMAKE_SOURCE_DIR}/3rdparty/onnxruntime (with include/ and lib/)")
    endif()
elseif(UNIX)
    if(EXISTS "${ONNXRUNTIME_ROOT}/include/onnxruntime_cxx_api.h")
        set(_FLYKYLIN_ONNXRUNTIME_LIB "")
        if(EXISTS "${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so")
            set(_FLYKYLIN_ONNXRUNTIME_LIB "${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so")
        elseif(EXISTS "${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so.1")
            set(_FLYKYLIN_ONNXRUNTIME_LIB "${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so.1")
        elseif(EXISTS "${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so.1.23.2")
            set(_FLYKYLIN_ONNXRUNTIME_LIB "${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so.1.23.2")
        endif()

        if(_FLYKYLIN_ONNXRUNTIME_LIB)
            message(STATUS "Found ONNX Runtime (Linux): ${ONNXRUNTIME_ROOT}")
            set(ONNXRUNTIME_FOUND TRUE)
            link_directories(${ONNXRUNTIME_ROOT}/lib)
        else()
            message(WARNING "ONNX Runtime headers found but no Linux .so in ${ONNXRUNTIME_ROOT}/lib; disabling ONNX Runtime on this platform")
        endif()
    endif()
endif()

# ONNX模型路径
set(NSFW_MODEL_PATH "E:/Project/tensorflow-open_nsfw/open_nsfw.onnx" CACHE PATH "NSFW detection model")
set(BGE_MODEL_PATH "E:/Project/tensorflow-open_nsfw/onnx_models/bge-small-zh-v1.5/bge-small-zh-v1.5.onnx" CACHE PATH "BGE semantic search model")
message(STATUS "NSFW Model: ${NSFW_MODEL_PATH}")
message(STATUS "BGE Model: ${BGE_MODEL_PATH}")

# RKNPU (仅RK3566)
if(IS_RK3566)
    set(RKNPU_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/rknn-toolkit2-2.3.2/rknpu2/runtime/Linux/librknn_api" CACHE PATH "RKNPU root directory")
    if(EXISTS "${RKNPU_ROOT}/include/rknn_api.h" AND EXISTS "${RKNPU_ROOT}/aarch64/librknnrt.so")
        message(STATUS "Found RKNPU: ${RKNPU_ROOT}")
        set(RKNPU_FOUND TRUE)
    else()
        message(WARNING "RKNPU not found in ${RKNPU_ROOT}")
        set(RKNPU_FOUND FALSE)
    endif()
endif()

# 版本信息配置
string(TIMESTAMP FLYKYLIN_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP FLYKYLIN_BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

# 获取 Git 提交哈希
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE FLYKYLIN_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()
if(NOT FLYKYLIN_GIT_HASH)
    set(FLYKYLIN_GIT_HASH "unknown")
endif()

# 生成版本头文件
configure_file(
    ${CMAKE_SOURCE_DIR}/src/core/Version.h.in
    ${CMAKE_BINARY_DIR}/generated/Version.h
    @ONLY
)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/platform
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_BINARY_DIR}/generated
)

# 先构建protocol库
add_subdirectory(protocol)

if(ONNXRUNTIME_FOUND)
    include_directories(${ONNXRUNTIME_ROOT}/include)
endif()

if(RKNPU_FOUND)
    include_directories(${RKNPU_ROOT}/include)
    link_directories(${RKNPU_ROOT}/aarch64)
endif()

# 源文件目录
add_subdirectory(src)
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 安装规则
install(TARGETS FlyKylin
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# CPack配置（打包）
set(CPACK_PACKAGE_NAME "FlyKylin")
set(CPACK_PACKAGE_VENDOR "FlyKylin Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "AI-Powered P2P LAN Communication Tool")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "FlyKylin")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "FlyKylin ${PROJECT_VERSION}")
    set(CPACK_NSIS_PACKAGE_NAME "FlyKylin")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
elseif(UNIX)
    set(CPACK_GENERATOR "DEB;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "FlyKylin Team")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qtbase5-dev | qt6-base, libprotobuf23")
endif()

include(CPack)

if(QT_VERSION_MAJOR EQUAL 6)
    set(QT_VERSION_STRING ${Qt6_VERSION})
elseif(QT_VERSION_MAJOR EQUAL 5)
    set(QT_VERSION_STRING ${Qt5_VERSION})
else()
    set(QT_VERSION_STRING "Unknown")
endif()

# 打印配置摘要
message(STATUS "========================================")
message(STATUS "  FlyKylin Configuration Summary")
message(STATUS "========================================")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Platform: ${PLATFORM_NAME}")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt Version: ${QT_VERSION_STRING}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  ONNX Runtime: ${ONNXRUNTIME_FOUND}")
if(IS_RK3566)
message(STATUS "  RKNPU: ${RKNPU_FOUND}")
endif()
message(STATUS "========================================")
