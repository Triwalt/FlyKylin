/**
 * 在线用户列表组件 (QML)
 * 
 * 替代 PeerListWidget.cpp/.h，代码量减少60%
 */

/**
 * 在线用户列表组件 (QML)
 * 
 * 替代 PeerListWidget.cpp/.h，代码量减少60%
 */

import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import "."

Item {
    id: peerList
    
    // 外部属性
    property var viewModel
    // 标题文本，可根据左侧选项卡显示“会话/在线用户”等
    property string titleText: qsTr("在线用户")
    // 是否使用包含离线用户的会话模型
    property bool useSessionModel: false
    // 待删除会话的用户信息
    property string pendingDeleteUserId: ""
    property string pendingDeleteUserName: ""

    AvatarLibrary {
        id: avatarLib
    }

    // 删除会话确认对话框：使用 QtQuick.Controls 2 的 Dialog，避免依赖 QtQuick.Dialogs
    Dialog {
        id: deleteSessionDialog
        modal: true
        title: qsTr("删除会话")

        contentItem: ColumnLayout {
            spacing: Style.spacingMedium

            Label {
                text: qsTr("确定要删除与 %1 的会话并清空聊天记录吗？").arg(peerList.pendingDeleteUserName)
                wrapMode: Text.WordWrap
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight

                Button {
                    text: qsTr("取消")
                    onClicked: {
                        peerList.pendingDeleteUserId = ""
                        peerList.pendingDeleteUserName = ""
                        deleteSessionDialog.close()
                    }
                }

                Button {
                    text: qsTr("确定")
                    onClicked: {
                        if (viewModel && peerList.pendingDeleteUserId !== "") {
                            viewModel.deleteSession(peerList.pendingDeleteUserId)
                        }
                        if (typeof chatViewModel !== "undefined" && chatViewModel
                                && peerList.pendingDeleteUserId !== "") {
                            chatViewModel.deleteConversation(peerList.pendingDeleteUserId)
                        }
                        peerList.pendingDeleteUserId = ""
                        peerList.pendingDeleteUserName = ""
                        deleteSessionDialog.close()
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Style.spacingMedium
        spacing: Style.spacingMedium
        
        // 标题栏：文本标题
        Label {
            Layout.fillWidth: true
            text: titleText + " (" + (viewModel
                                       ? (useSessionModel
                                          ? viewModel.sessionCount
                                          : viewModel.onlineCount)
                                       : 0) + ")"
            font: Style.fontHeading
            color: Style.textPrimary
        }
        
        // 搜索框
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: Style.background
            radius: Style.radiusSmall
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8
                
                // Icon placeholder
                Label {
                    text: "\ue8b6" // search icon
                    font: Style.iconFont
                    color: Style.textSecondary
                }
                
                TextField {
                    id: searchField
                    Layout.fillWidth: true
                    placeholderText: qsTr("搜索用户...")
                    background: null
                    font: Style.fontBody
                    color: Style.textPrimary
                    
                    onTextChanged: {
                        if (viewModel) {
                            viewModel.filterKeyword = text
                        }
                    }
                }
            }
        }
        
        // 用户列表
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            model: viewModel
                   ? (useSessionModel ? viewModel.sessionListModel : viewModel.peerListModel)
                   : null
            clip: true
            spacing: 4
            
            delegate: ItemDelegate {
                width: listView.width
                height: 72 // Taller items
                
                contentItem: RowLayout {
                    spacing: Style.spacingMedium
                    
                    // 头像
                    Rectangle {
                        width: 48
                        height: 48
                        radius: 24
                        color: model.userId === settingsViewModel.localUserId
                               ? avatarLib.colorFor(settingsViewModel.avatarId)
                               : avatarLib.colorForName(model.userName || model.userId)
                        
                        Label {
                            anchors.centerIn: parent
                            text: model.userName ? model.userName.charAt(0).toUpperCase() : "?"
                            color: "white"
                            font: Style.fontTitle
                        }
                        
                        // 在线状态指示器 (Badge)
                        Rectangle {
                            width: 14
                            height: 14
                            radius: 7
                            color: model.isOnline ? Style.online : Style.offline
                            border.width: 2
                            border.color: Style.surface
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                        }
                    }
                    
                    // 用户信息
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: model.userName || ""
                            font: Style.fontTitle
                            color: Style.textPrimary
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            text: {
                                var addr = model.ipAddress || "";
                                var portText = "";
                                if (model.tcpPort && model.tcpPort > 0) {
                                    portText = ":" + model.tcpPort;
                                }
                                return addr + portText;
                            }
                            font: Style.fontCaption
                            color: Style.textSecondary
                            elide: Text.ElideRight
                        }
                    }
                }
                
                // 悬停和选中效果
                background: Rectangle {
                    color: parent.hovered || parent.down ? Style.surfaceHover : "transparent"
                    radius: Style.radiusSmall
                    
                    // Active indicator
                    Rectangle {
                        visible: parent.down || (viewModel && viewModel.currentPeerId === model.userId) // Need currentPeerId in VM
                        width: 4
                        height: 24
                        radius: 2
                        color: Style.primary
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                
                onClicked: {
                    if (viewModel) {
                        viewModel.selectPeer(model.userId)
                    }
                }

                Menu {
                    id: contextMenu
                    MenuItem {
                        text: qsTr("查看信息")
                        onTriggered: {
                            if (viewModel) {
                                viewModel.requestPeerDetails(model.userId)
                            }
                        }
                    }
                    MenuSeparator {}
                    MenuItem {
                        text: qsTr("加入联系人列表")
                        onTriggered: {
                            if (viewModel) {
                                viewModel.requestAddToContacts(model.userId)
                            }
                        }
                    }
                    MenuItem {
                        text: qsTr("加入群聊")
                        onTriggered: {
                            if (viewModel) {
                                viewModel.requestAddToGroup(model.userId)
                            }
                        }
                    }
                    MenuSeparator {
                        visible: peerList.useSessionModel
                    }
                    MenuItem {
                        text: qsTr("删除会话")
                        visible: peerList.useSessionModel
                        onTriggered: {
                            peerList.pendingDeleteUserId = model.userId
                            peerList.pendingDeleteUserName = model.userName || model.userId
                            deleteSessionDialog.open()
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    onClicked: function(mouse) {
                        if (mouse.button === Qt.RightButton) {
                            contextMenu.open()
                            mouse.accepted = true
                        }
                    }
                }
            }
            
            ScrollBar.vertical: ScrollBar {}
        }
    }
}
