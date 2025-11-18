cmake_minimum_required(VERSION 3.20)

project(FlyKylin 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "AI-Powered P2P LAN Communication Tool"
)

# C++20标准
set(CMAKE_CXX_STANDARD 20)
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
    
    # 检测是否为ARM64 (RK3566)
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(ARCH STREQUAL "aarch64")
        set(IS_RK3566 ON)
        add_compile_definitions(RK3566_PLATFORM)
        message(STATUS "Building for RK3566 ARM64 platform")
    endif()
endif()

message(STATUS "Building for ${PLATFORM_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

# 查找依赖
# Qt 6.9.3路径 (D:/Qt/6.9.3/msvc2022_64)
set(CMAKE_PREFIX_PATH "D:/Qt/6.9.3/msvc2022_64" CACHE PATH "Qt installation path")

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Sql
    Concurrent
    Test
)

# Protobuf (optional for now)
find_package(Protobuf)
if(NOT Protobuf_FOUND)
    message(WARNING "Protobuf not found - some features will be disabled")
    set(ENABLE_PROTOBUF FALSE)
else()
    set(ENABLE_PROTOBUF TRUE)
    message(STATUS "Protobuf found: ${Protobuf_VERSION}")
endif()

# ONNX Runtime
# 注意：Python环境中有onnxruntime 1.23.2，C++ API需要单独下载
# 从 https://github.com/microsoft/onnxruntime/releases 下载预编译库
set(ONNXRUNTIME_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/onnxruntime" CACHE PATH "ONNX Runtime root directory")
if(EXISTS "${ONNXRUNTIME_ROOT}/include")
    message(STATUS "Found ONNX Runtime: ${ONNXRUNTIME_ROOT}")
    set(ONNXRUNTIME_FOUND TRUE)
    # 添加库路径
    link_directories(${ONNXRUNTIME_ROOT}/lib)
else()
    message(WARNING "ONNX Runtime C++ library not found in ${ONNXRUNTIME_ROOT}")
    message(WARNING "Please download from: https://github.com/microsoft/onnxruntime/releases")
    message(WARNING "Extract to: ${CMAKE_SOURCE_DIR}/3rdparty/onnxruntime")
    set(ONNXRUNTIME_FOUND FALSE)
endif()

# ONNX模型路径
set(NSFW_MODEL_PATH "E:/Project/tensorflow-open_nsfw/open_nsfw.onnx" CACHE PATH "NSFW detection model")
set(BGE_MODEL_PATH "E:/Project/tensorflow-open_nsfw/onnx_models/bge-small-zh-v1.5/bge-small-zh-v1.5.onnx" CACHE PATH "BGE semantic search model")
message(STATUS "NSFW Model: ${NSFW_MODEL_PATH}")
message(STATUS "BGE Model: ${BGE_MODEL_PATH}")

# RKNPU (仅RK3566)
if(IS_RK3566)
    set(RKNPU_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/rknn_api" CACHE PATH "RKNPU root directory")
    if(EXISTS "${RKNPU_ROOT}/include")
        message(STATUS "Found RKNPU: ${RKNPU_ROOT}")
        set(RKNPU_FOUND TRUE)
    else()
        message(WARNING "RKNPU not found in ${RKNPU_ROOT}")
        set(RKNPU_FOUND FALSE)
    endif()
endif()

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/platform
    ${CMAKE_SOURCE_DIR}/src/ui
    ${Protobuf_INCLUDE_DIRS}
)

if(ONNXRUNTIME_FOUND)
    include_directories(${ONNXRUNTIME_ROOT}/include)
endif()

if(RKNPU_FOUND)
    include_directories(${RKNPU_ROOT}/include)
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
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base, libprotobuf23")
endif()

include(CPack)

# 打印配置摘要
message(STATUS "========================================")
message(STATUS "  FlyKylin Configuration Summary")
message(STATUS "========================================")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Platform: ${PLATFORM_NAME}")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt Version: ${Qt6_VERSION}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  ONNX Runtime: ${ONNXRUNTIME_FOUND}")
if(IS_RK3566)
message(STATUS "  RKNPU: ${RKNPU_FOUND}")
endif()
message(STATUS "========================================")
