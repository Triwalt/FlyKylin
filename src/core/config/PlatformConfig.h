/**
 * @file PlatformConfig.h
 * @brief 统一平台配置头文件
 * @author FlyKylin Team
 * @date 2025-12-06
 * 
 * 集中管理平台检测、Qt版本判断和功能可用性宏。
 * 使用方法：在需要平台差异化处理的源文件中 #include 此头文件。
 */

#ifndef FLYKYLIN_CORE_CONFIG_PLATFORMCONFIG_H
#define FLYKYLIN_CORE_CONFIG_PLATFORMCONFIG_H

#include <QtGlobal>

// ============================================================================
// 平台检测宏
// ============================================================================

#if defined(Q_OS_WIN)
    #define FLYKYLIN_PLATFORM_WINDOWS 1
    #define FLYKYLIN_PLATFORM_NAME "Windows"
#elif defined(Q_OS_LINUX)
    #define FLYKYLIN_PLATFORM_LINUX 1
    #define FLYKYLIN_PLATFORM_NAME "Linux"
    
    // RK3566 ARM64 嵌入式平台检测
    #if defined(FLYKYLIN_FORCE_RK3566) || defined(RK3566_PLATFORM)
        #define FLYKYLIN_PLATFORM_RK3566 1
        #define FLYKYLIN_PLATFORM_EMBEDDED 1
    #endif
#elif defined(Q_OS_MACOS)
    #define FLYKYLIN_PLATFORM_MACOS 1
    #define FLYKYLIN_PLATFORM_NAME "macOS"
#else
    #define FLYKYLIN_PLATFORM_UNKNOWN 1
    #define FLYKYLIN_PLATFORM_NAME "Unknown"
#endif

// ============================================================================
// 架构检测宏
// ============================================================================

#if defined(Q_PROCESSOR_X86_64)
    #define FLYKYLIN_ARCH_AMD64 1
    #define FLYKYLIN_ARCH_NAME "amd64"
#elif defined(Q_PROCESSOR_X86_32)
    #define FLYKYLIN_ARCH_X86 1
    #define FLYKYLIN_ARCH_NAME "x86"
#elif defined(Q_PROCESSOR_ARM_64)
    #define FLYKYLIN_ARCH_ARM64 1
    #define FLYKYLIN_ARCH_NAME "arm64"
#elif defined(Q_PROCESSOR_ARM)
    #define FLYKYLIN_ARCH_ARM32 1
    #define FLYKYLIN_ARCH_NAME "arm"
#else
    #define FLYKYLIN_ARCH_UNKNOWN 1
    #define FLYKYLIN_ARCH_NAME "unknown"
#endif

// ============================================================================
// Qt 版本检测宏
// ============================================================================

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #define FLYKYLIN_QT6 1
    #define FLYKYLIN_QT_MAJOR 6
#else
    #define FLYKYLIN_QT5 1
    #define FLYKYLIN_QT_MAJOR 5
#endif

// Qt 版本特性检测
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    #define FLYKYLIN_HAS_QT64_FEATURES 1
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    #define FLYKYLIN_HAS_QT515_FEATURES 1
#endif

// ============================================================================
// 功能可用性宏
// ============================================================================

// QtConcurrent 仅在非嵌入式平台可用
#if !defined(FLYKYLIN_PLATFORM_RK3566) && !defined(FLYKYLIN_PLATFORM_EMBEDDED)
    #define FLYKYLIN_HAS_CONCURRENT 1
#endif

// ONNX Runtime AI 功能
#if defined(FLYKYLIN_ENABLE_ONNXRUNTIME) || defined(ONNXRUNTIME_FOUND)
    #define FLYKYLIN_HAS_AI_FEATURES 1
    #define FLYKYLIN_HAS_ONNX 1
#endif

// RKNPU 加速功能 (仅 RK3566)
#if defined(FLYKYLIN_PLATFORM_RK3566) && defined(RKNPU_FOUND)
    #define FLYKYLIN_HAS_RKNPU 1
#endif

// ============================================================================
// 编译器特性检测
// ============================================================================

#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
    #define FLYKYLIN_HAS_CONCEPTS 1
#endif

#if defined(__cpp_coroutines) || defined(__cpp_impl_coroutine)
    #define FLYKYLIN_HAS_COROUTINES 1
#endif

// ============================================================================
// 辅助宏
// ============================================================================

/// 平台特定代码块
#define FLYKYLIN_PLATFORM_BLOCK_BEGIN(platform) \
    if constexpr (FLYKYLIN_PLATFORM_##platform) {
#define FLYKYLIN_PLATFORM_BLOCK_END }

/// 条件编译日志（仅调试模式）
#ifdef QT_DEBUG
    #define FLYKYLIN_PLATFORM_LOG() \
        qDebug() << "[Platform]" << FLYKYLIN_PLATFORM_NAME << FLYKYLIN_ARCH_NAME << "Qt" << FLYKYLIN_QT_MAJOR
#else
    #define FLYKYLIN_PLATFORM_LOG() ((void)0)
#endif

#endif // FLYKYLIN_CORE_CONFIG_PLATFORMCONFIG_H
