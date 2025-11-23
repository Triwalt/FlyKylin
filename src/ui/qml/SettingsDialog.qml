import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

Item {
    id: root
    anchors.fill: parent
    visible: false
    z: 100

    // Dimmed background
    Rectangle {
        anchors.fill: parent
        color: "#80000000"

        MouseArea {
            anchors.fill: parent
            onClicked: root.visible = false
        }
    }

    Rectangle {
        id: panel
        width: 520
        height: 560
        radius: Style.radiusLarge
        color: Style.surface
        anchors.centerIn: parent

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Style.spacingLarge
            spacing: Style.spacingMedium

            // Header
            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: qsTr("设置")
                    font: Style.fontHeading
                    color: Style.textPrimary
                    Layout.fillWidth: true
                }

                Button {
                    implicitWidth: 32
                    implicitHeight: 32
                    background: Rectangle {
                        anchors.fill: parent
                        radius: 16
                        color: Style.background
                        Label {
                            anchors.centerIn: parent
                            text: "\ue5cd" // close icon
                            font: Style.iconFont
                            color: Style.textSecondary
                        }
                    }
                    onClicked: root.visible = false
                }
            }

            // Personal info
            GroupBox {
                title: qsTr("个人信息")
                Layout.fillWidth: true
                leftPadding: Style.spacingLarge
                rightPadding: Style.spacingLarge
                topPadding: Style.spacingMedium
                bottomPadding: Style.spacingMedium

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 0
                    spacing: Style.spacingSmall

                    Label {
                        text: qsTr("用户名")
                        font: Style.fontCaption
                        color: Style.textSecondary
                    }

                    TextField {
                        Layout.fillWidth: true
                        text: settingsViewModel ? settingsViewModel.userName : ""
                        font: Style.fontBody
                        onEditingFinished: if (settingsViewModel) settingsViewModel.userName = text
                    }

                    Label {
                        text: qsTr("头像")
                        font: Style.fontCaption
                        color: Style.textSecondary
                        topPadding: Style.spacingMedium
                    }

                    // Avatar choices
                    AvatarLibrary {
                        id: avatarLib
                    }

                    GridLayout {
                        columns: 6
                        rowSpacing: Style.spacingSmall
                        columnSpacing: Style.spacingSmall

                        Repeater {
                            model: avatarLib.avatars

                            delegate: Rectangle {
                                width: 40
                                height: 40
                                radius: 20
                                color: modelData.color
                                border.width: settingsViewModel.avatarId === modelData.id ? 3 : 0
                                border.color: Style.primary

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: settingsViewModel.avatarId = modelData.id
                                }
                            }
                        }
                    }
                }
            }

            // Storage settings
            GroupBox {
                title: qsTr("文件与存储")
                Layout.fillWidth: true
                leftPadding: Style.spacingLarge
                rightPadding: Style.spacingLarge
                topPadding: Style.spacingMedium
                bottomPadding: Style.spacingMedium

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 0
                    spacing: Style.spacingSmall

                    // Download directory
                    Label {
                        text: qsTr("下载目录")
                        font: Style.fontCaption
                        color: Style.textSecondary
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        TextField {
                            Layout.fillWidth: true
                            text: settingsViewModel ? settingsViewModel.downloadDirectory : ""
                            readOnly: true
                            font: Style.fontBody
                        }

                        Button {
                            text: qsTr("选择...")
                            onClicked: settingsViewModel.chooseDownloadDirectory()
                        }
                    }

                    // Chat history directory (reserved for future use)
                    Label {
                        text: qsTr("聊天记录目录 (预留)")
                        font: Style.fontCaption
                        color: Style.textSecondary
                        topPadding: Style.spacingMedium
                    }

                    TextField {
                        Layout.fillWidth: true
                        text: settingsViewModel ? settingsViewModel.chatHistoryDirectory : ""
                        font: Style.fontBody
                        onEditingFinished: if (settingsViewModel) settingsViewModel.chatHistoryDirectory = text
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
