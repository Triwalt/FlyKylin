/**
 * @file PlatformQmlConfig.qml
 * @brief 平台特定 QML 配置 (根据编译目标自动切换)
 * @author FlyKylin Team
 * @date 2025-12-06
 * 
 * 使用方法: import "."; PlatformConfig.imageSourceSize
 * 
 * 在 CMake 中通过 configure_file 或 QML_IMPORT_PATH 注入平台常量
 */

pragma Singleton
import QtQuick 2.12

QtObject {
    // 平台标识 (由 C++ 注入，此处为默认值)
    readonly property bool isEmbedded: Qt.platform.os !== "windows" && Qt.platform.os !== "osx"
    readonly property bool isRK3566: typeof __RK3566_PLATFORM !== "undefined" ? __RK3566_PLATFORM : false
    
    // ========================================
    // 图片配置 - 根据平台动态调整
    // ========================================
    
    // 聊天图片缩略图尺寸 (sourceSize)
    readonly property int imageSourceWidth: isRK3566 ? 260 : 1024
    readonly property int imageSourceHeight: isRK3566 ? 260 : 1024
    
    // 图片显示最大宽度
    readonly property int imageDisplayMaxWidth: isRK3566 ? 200 : 260
    
    // 是否启用图片缓存 (嵌入式设备建议开启以减少重复解码)
    readonly property bool imageCacheEnabled: isRK3566 ? true : false
    
    // 自定义表情包缩略图尺寸
    readonly property int emojiThumbnailSize: isRK3566 ? 48 : 64
    
    // ========================================
    // 列表配置 - 嵌入式设备减少内存占用
    // ========================================
    
    // ListView 显示边距 (减少可视范围外的预加载)
    readonly property int listDisplayMargin: isRK3566 ? 20 : 40
    
    // 消息历史加载数量
    readonly property int messagePageSize: isRK3566 ? 15 : 20
    
    // ========================================
    // 动画配置 - 嵌入式设备简化动画
    // ========================================
    
    readonly property bool enableAnimations: !isRK3566
    readonly property int animationDuration: isRK3566 ? 100 : 250
}
