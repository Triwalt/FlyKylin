/**
 * 聊天视图组件 (QML)
 */

/**
 * 聊天视图组件 (QML)
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import QtQuick.Dialogs
import "."

Item {
    id: chatView
    
    property var viewModel
    
    AvatarLibrary {
        id: avatarLib
    }
    
    // Background
    Rectangle {
        anchors.fill: parent
        color: Style.background
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: Style.surface
            
            // Shadow effect
            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowColor: "#10000000"
                shadowBlur: 10
                shadowVerticalOffset: 2
            }
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: Style.spacingMedium
                spacing: Style.spacingMedium
                
                // Avatar placeholder
                Rectangle {
                    width: 40
                    height: 40
                    radius: 20
                    color: avatarLib.colorForName(viewModel ? viewModel.currentPeerName : "")
                    visible: viewModel && viewModel.currentPeerName !== ""
                    
                    Label {
                        anchors.centerIn: parent
                        text: viewModel && viewModel.currentPeerName ? viewModel.currentPeerName.charAt(0).toUpperCase() : ""
                        color: "white"
                        font: Style.fontTitle
                    }
                }
                
                ColumnLayout {
                    spacing: 2
                    
                    Label {
                        text: viewModel && viewModel.currentPeerName ? viewModel.currentPeerName : qsTr("未选择用户")
                        font: Style.fontHeading
                        color: Style.textPrimary
                    }
                    
                    Label {
                        text: viewModel && viewModel.currentPeerName ? qsTr("在线") : ""
                        font: Style.fontCaption
                        color: Style.online
                        visible: viewModel && viewModel.currentPeerName !== ""
                    }
                }
            }
        }

        // Messages
        ListView {
            id: messageList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: viewModel ? viewModel.messageModel : null
            spacing: Style.spacingMedium
            displayMarginBeginning: 40
            displayMarginEnd: 40
            
            // Padding
            topMargin: Style.spacingMedium
            bottomMargin: Style.spacingMedium
            leftMargin: Style.spacingLarge
            rightMargin: Style.spacingLarge
            
            // Auto scroll to bottom
            onCountChanged: {
                Qt.callLater(function() { positionViewAtEnd() })
            }

            delegate: Item {
                id: messageDelegate
                width: messageList.width
                height: bubbleItem.childrenRect.height

                Item {
                    id: bubbleItem
                    width: parent.width - 2 * Style.spacingLarge
                    height: childrenRect.height
                    anchors.horizontalCenter: parent.horizontalCenter

                    // Bubble
                    Rectangle {
                        id: msgContent
                        // Message types: text / image / file
                        property bool isImage: model.kind === "image"
                        property bool isFile: model.kind === "file"
                        property bool isText: !isImage && !isFile

                        // Width is at most 80% of available row width, with inner padding
                        width: Math.min(parent.width * 0.8, contentColumn.implicitWidth + 32)
                        height: contentColumn.implicitHeight + 24

                        anchors.right: model.isMine ? parent.right : undefined
                        anchors.left: model.isMine ? undefined : parent.left
                        anchors.rightMargin: Style.spacingMedium
                        anchors.leftMargin: Style.spacingMedium

                        color: model.isMine ? Style.primary : Style.surface
                        radius: Style.radiusMedium

                        Column {
                            id: contentColumn
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 6

                            // Sender avatar + name (Telegram-style header per bubble)
                            Item {
                                id: senderRow
                                width: parent.width
                                height: 24

                                Rectangle {
                                    id: senderAvatar
                                    width: 24
                                    height: 24
                                    radius: 12
                                    color: model.isMine
                                           ? avatarLib.colorFor(settingsViewModel.avatarId)
                                           : avatarLib.colorForName(model.senderName || (viewModel ? viewModel.currentPeerName : ""))
                                    anchors.left: parent.left
                                    anchors.verticalCenter: parent.verticalCenter

                                    Label {
                                        anchors.centerIn: parent
                                        text: model.isMine
                                              ? avatarLib.initialForName(settingsViewModel.userName)
                                              : avatarLib.initialForName(model.senderName || (viewModel ? viewModel.currentPeerName : ""))
                                        color: "white"
                                        font: Style.fontCaption
                                    }
                                }

                                Label {
                                    anchors.left: senderAvatar.right
                                    anchors.leftMargin: 8
                                    anchors.bottom: senderAvatar.bottom
                                    width: parent.width - senderAvatar.width - 8
                                    text: model.isMine
                                          ? settingsViewModel.userName
                                          : (model.senderName || (viewModel ? viewModel.currentPeerName : ""))
                                    font: Style.fontCaption
                                    color: model.isMine ? Style.textOnPrimary : Style.textSecondary
                                    elide: Text.ElideRight
                                }
                            }

                            // Text message content
                            Label {
                                id: msgText
                                visible: msgContent.isText
                                width: parent.width
                                text: model.content
                                color: model.isMine ? Style.textOnPrimary : Style.textPrimary
                                font: Style.fontBody
                                wrapMode: Text.Wrap
                            }

                            // File attachment (non-image)
                            Row {
                                id: fileRow
                                visible: msgContent.isFile
                                spacing: 8
                                anchors.horizontalCenter: parent.horizontalCenter

                                Rectangle {
                                    width: 28
                                    height: 28
                                    radius: 6
                                    color: model.isMine ? Style.primaryLight : "#E5E7EB"

                                    Label {
                                        anchors.centerIn: parent
                                        text: "\ue2c4" // attach file icon
                                        font: Style.iconFont
                                        color: model.isMine ? "white" : Style.textSecondary
                                    }
                                }

                                Column {
                                    spacing: 2

                                    Label {
                                        text: model.attachmentName || model.content
                                        font: Style.fontBody
                                        color: model.isMine ? Style.textOnPrimary : Style.textPrimary
                                        elide: Text.ElideRight
                                        width: parent.width
                                    }

                                    Label {
                                        text: qsTr("文件")
                                        font: Style.fontCaption
                                        color: Style.textSecondary
                                    }
                                }
                            }

                            // Image attachment thumbnail
                            Image {
                                id: msgImage
                                visible: msgContent.isImage
                                anchors.horizontalCenter: parent.horizontalCenter
                                source: model.attachmentPath && model.attachmentPath !== "" ?
                                            (model.attachmentPath.startsWith("file:")
                                                 ? model.attachmentPath
                                                 : "file:///" + model.attachmentPath)
                                            : ""
                                fillMode: Image.PreserveAspectFit
                                asynchronous: true
                                cache: true
                                width: Math.min(260, parent.width)
                            }
                        }

                        // Shadow for received messages only
                        layer.enabled: !model.isMine
                        layer.effect: MultiEffect {
                            shadowEnabled: true
                            shadowColor: "#08000000"
                            shadowBlur: 4
                            shadowVerticalOffset: 2
                        }
                    }

                    // Timestamp directly under bubble, aligned with bubble edge
                    Label {
                        id: timestampLabel
                        anchors.top: msgContent.bottom
                        anchors.topMargin: 4
                        anchors.right: model.isMine ? msgContent.right : undefined
                        anchors.left: model.isMine ? undefined : msgContent.left
                        text: model.timestamp
                        font: Style.fontCaption
                        color: Style.textSecondary
                        opacity: 0.8
                    }
                }
            }
        }

        // Input Area
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: Style.surface
            
            // Top border
            Rectangle {
                width: parent.width
                height: 1
                color: "#E5E7EB"
                anchors.top: parent.top
            }
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: Style.spacingMedium
                spacing: Style.spacingMedium

                // Image send button
                Button {
                    id: imageBtn
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40

                    background: Rectangle {
                        color: parent.enabled ? Style.surface : Style.background
                        radius: 20

                        Label {
                            anchors.centerIn: parent
                            text: "\ue3f4" // image icon
                            font: Style.iconFont
                            color: Style.textSecondary
                        }
                    }

                    enabled: viewModel && viewModel.currentPeerName !== ""
                    onClicked: imageFileDialog.open()
                }

                // File send button
                Button {
                    id: fileBtn
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40

                    background: Rectangle {
                        color: parent.enabled ? Style.surface : Style.background
                        radius: 20

                        Label {
                            anchors.centerIn: parent
                            text: "\ue2c4" // attach file icon
                            font: Style.iconFont
                            color: Style.textSecondary
                        }
                    }

                    enabled: viewModel && viewModel.currentPeerName !== ""
                    onClicked: fileDialog.open()
                }

                // Input Box
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: Style.background
                    radius: Style.radiusLarge
                    border.width: 1
                    border.color: inputField.activeFocus ? Style.primary : "transparent"
                    
                    TextField {
                        id: inputField
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        verticalAlignment: Text.AlignVCenter
                        
                        placeholderText: qsTr("输入消息...")
                        font: Style.fontBody
                        color: Style.textPrimary
                        background: null
                        selectByMouse: true
                        
                        enabled: viewModel && viewModel.currentPeerName !== ""
                        onAccepted: sendBtn.clicked()
                    }
                }
                
                // Send Button
                Button {
                    id: sendBtn
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48
                    
                    background: Rectangle {
                        color: parent.enabled ? Style.primary : Style.background
                        radius: 24
                        
                        Label {
                            anchors.centerIn: parent
                            text: "\ue163" // send icon
                            font: Style.iconFont
                            color: parent.parent.enabled ? "white" : Style.textSecondary
                        }
                    }
                    
                    enabled: inputField.text.length > 0
                    onClicked: {
                        if (viewModel) {
                            viewModel.sendMessage(inputField.text)
                            inputField.text = ""
                        }
                    }
                }
            }
        }
    }

    // Image picker dialog
    FileDialog {
        id: imageFileDialog
        title: qsTr("选择图片")
        nameFilters: [qsTr("Images (*.png *.jpg *.jpeg *.bmp *.gif)")]
        onAccepted: {
            if (viewModel && selectedFile !== "") {
                viewModel.sendImage(selectedFile)
            }
        }
    }

    // Generic file picker dialog
    FileDialog {
        id: fileDialog
        title: qsTr("选择文件")
        onAccepted: {
            if (viewModel && selectedFile !== "") {
                viewModel.sendFile(selectedFile)
            }
        }
    }
}
