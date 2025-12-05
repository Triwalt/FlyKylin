pragma Singleton
import QtQuick 2.12

QtObject {
    // Brand Colors
    readonly property color primary: "#6366F1" // Indigo
    readonly property color primaryDark: "#4F46E5"
    readonly property color primaryLight: "#818CF8"
    
    // Background Colors
    readonly property color background: "#F3F4F6" // Cool Gray 100
    readonly property color surface: "#FFFFFF"
    readonly property color surfaceHover: "#F9FAFB"
    
    // Text Colors
    readonly property color textPrimary: "#111827" // Gray 900
    readonly property color textSecondary: "#6B7280" // Gray 500
    readonly property color textOnPrimary: "#FFFFFF"
    
    // Status Colors
    readonly property color online: "#10B981" // Emerald
    readonly property color offline: "#9CA3AF" // Gray 400
    
    // Border Radius
    readonly property int radiusSmall: 8
    readonly property int radiusMedium: 12
    readonly property int radiusLarge: 16
    
    // Spacing System
    readonly property int spacingXs: 4
    readonly property int spacingSmall: 8
    readonly property int spacingSm: 8  // alias
    readonly property int spacingMedium: 16
    readonly property int spacingMd: 16 // alias
    readonly property int spacingLarge: 24
    readonly property int spacingLg: 24 // alias
    readonly property int spacingXl: 32
    
    // ========================================
    // Layout Dimensions (NEW)
    // ========================================
    
    // Sidebar and Navigation
    readonly property int sidebarWidth: 320       // Main sidebar width
    readonly property int iconBarWidth: 72        // Left icon navigation bar
    readonly property int chatListWidth: 248      // Chat list within sidebar (sidebarWidth - iconBarWidth)
    
    // Chat Area
    readonly property int minChatWidth: 400       // Minimum chat panel width
    readonly property int inputAreaHeight: 120    // Message input area height
    readonly property int avatarSize: 40          // User avatar size
    readonly property int avatarSizeSmall: 32     // Small avatar size
    
    // Tab/Button Heights
    readonly property int tabHeight: 56           // Side tab button height
    readonly property int buttonHeight: 40        // Standard button height
    readonly property int headerHeight: 64        // Page header height
    
    // ========================================
    // Responsive Breakpoints (NEW)
    // ========================================
    
    readonly property int breakpointSm: 640       // Small screens
    readonly property int breakpointMd: 768       // Medium screens (tablets)
    readonly property int breakpointLg: 1024      // Large screens (desktop)
    readonly property int breakpointXl: 1280      // Extra large screens
    
    // ========================================
    // Animation Durations (NEW)
    // ========================================
    
    readonly property int animFast: 150           // Fast transitions
    readonly property int animNormal: 250         // Normal transitions
    readonly property int animSlow: 400           // Slow/emphasized transitions
    
    // Fonts
    readonly property string fontFamily: "Microsoft YaHei UI"
    
    readonly property font fontHeading: Qt.font({
        family: fontFamily,
        pixelSize: 20,
        weight: Font.Bold
    })
    
    readonly property font fontTitle: Qt.font({
        family: fontFamily,
        pixelSize: 16,
        weight: Font.DemiBold
    })
    
    readonly property font fontBody: Qt.font({
        family: fontFamily,
        pixelSize: 14,
        weight: Font.Normal
    })
    
    readonly property font fontCaption: Qt.font({
        family: fontFamily,
        pixelSize: 12,
        weight: Font.Normal
    })
    
    // Icon Font
    readonly property font iconFont: Qt.font({
        family: "Material Icons",
        pixelSize: 24,
        weight: Font.Normal
    })
}
