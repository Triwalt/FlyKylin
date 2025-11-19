/**
 * FlyKylin 主窗口 (QML)
 * 
 * 这是现代化重构的QML UI示例，将在Sprint 3全面实施
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FlyKylin.ViewModels 1.0

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1200
    height: 800
    title: qsTr("FlyKylin - AI智能飞秋")
    
    // 主布局
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // 左侧：在线用户列表
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 300
            color: "#f5f5f5"
            
            // 使用自定义组件
            PeerList {
                anchors.fill: parent
                viewModel: peerListViewModel
            }
        }
        
        // 右侧：聊天区域
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "white"
            
            // 聊天视图（待实现）
            ChatView {
                anchors.fill: parent
            }
        }
    }
    
    // ViewModel实例（在C++中注册）
    property var peerListViewModel
}
