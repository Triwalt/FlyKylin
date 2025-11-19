/**
 * 在线用户列表组件 (QML)
 * 
 * 替代 PeerListWidget.cpp/.h，代码量减少60%
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: peerList
    
    // 外部属性
    property var viewModel
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // 标题栏
        Label {
            text: qsTr("在线用户 (%1)").arg(viewModel ? viewModel.onlineCount : 0)
            font.bold: true
            font.pixelSize: 16
        }
        
        // 搜索框
        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("搜索用户...")
            
            onTextChanged: {
                if (viewModel) {
                    viewModel.filterKeyword = text
                }
            }
        }
        
        // 用户列表
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            model: viewModel ? viewModel.peerListModel : null
            clip: true
            
            delegate: ItemDelegate {
                width: listView.width
                height: 60
                
                contentItem: RowLayout {
                    spacing: 10
                    
                    // 头像（占位符）
                    Rectangle {
                        width: 40
                        height: 40
                        radius: 20
                        color: "#007AFF"
                        
                        Label {
                            anchors.centerIn: parent
                            text: model.userName ? model.userName.charAt(0) : "?"
                            color: "white"
                            font.bold: true
                        }
                    }
                    
                    // 用户信息
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        
                        Label {
                            text: model.userName || ""
                            font.bold: true
                            font.pixelSize: 14
                        }
                        
                        Label {
                            text: model.ipAddress || ""
                            font.pixelSize: 12
                            color: "#666666"
                        }
                    }
                    
                    // 在线状态指示器
                    Rectangle {
                        width: 10
                        height: 10
                        radius: 5
                        color: model.isOnline ? "#34C759" : "#8E8E93"
                    }
                }
                
                onClicked: {
                    // 选中用户
                    if (viewModel) {
                        // viewModel.selectPeer(model.userId)
                    }
                }
                
                // 悬停效果
                background: Rectangle {
                    color: parent.hovered ? "#e0e0e0" : "transparent"
                }
            }
            
            ScrollBar.vertical: ScrollBar {}
        }
        
        // 状态栏
        Label {
            text: qsTr("双击用户发起聊天")
            font.pixelSize: 12
            color: "#999999"
        }
    }
}
