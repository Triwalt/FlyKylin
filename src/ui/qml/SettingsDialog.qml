import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
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
        width: Math.min(parent.width - 40, 520)
        height: Math.min(parent.height - 40, 560)
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

            ScrollView {
                id: settingsScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ColumnLayout {
                    id: settingsContent
                    width: settingsScroll.width
                    spacing: Style.spacingMedium

                    // Personal info
                    GroupBox {
                        title: qsTr("个人信息")
                        Layout.fillWidth: true
                        leftPadding: Style.spacingLarge
                        rightPadding: Style.spacingLarge
                        topPadding: Style.spacingMedium
                        bottomPadding: Style.spacingMedium
                        background: Rectangle {
                            color: Style.surface
                            radius: 0
                            border.width: 1
                            border.color: "#E5E7EB"
                        }

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
                        background: Rectangle {
                            color: Style.surface
                            radius: 0
                            border.width: 1
                            border.color: "#E5E7EB"
                        }

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

                    // Search & experimental features
                    GroupBox {
                        title: qsTr("搜索与实验功能")
                        Layout.fillWidth: true
                        leftPadding: Style.spacingLarge
                        rightPadding: Style.spacingLarge
                        topPadding: Style.spacingMedium
                        bottomPadding: Style.spacingMedium
                        background: Rectangle {
                            color: Style.surface
                            radius: 0
                            border.width: 1
                            border.color: "#E5E7EB"
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 0
                            spacing: Style.spacingSmall

                            Label {
                                text: qsTr("语义搜索（词嵌入）")
                                font: Style.fontCaption
                                color: Style.textSecondary
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("开启后使用语义向量检索聊天记录，关闭时使用普通关键字搜索。")
                                    font: Style.fontCaption
                                    color: Style.textSecondary
                                    wrapMode: Text.Wrap
                                }

                                Switch {
                                    id: semanticSearchSwitch
                                    checked: settingsViewModel ? settingsViewModel.semanticSearchEnabled : false
                                    onToggled: if (settingsViewModel) settingsViewModel.semanticSearchEnabled = checked
                                }
                            }

                            Label {
                                text: qsTr("图片 NSFW 检测与拦截")
                                font: Style.fontCaption
                                color: Style.textSecondary
                                topPadding: Style.spacingMedium
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("发送图片前启用 NSFW 拦截")
                                    font: Style.fontCaption
                                    color: Style.textSecondary
                                    wrapMode: Text.Wrap
                                }

                                Switch {
                                    id: nsfwBlockOutgoingSwitch
                                    checked: settingsViewModel ? settingsViewModel.nsfwBlockOutgoing : false
                                    onToggled: if (settingsViewModel) settingsViewModel.nsfwBlockOutgoing = checked
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("接收图片时启用 NSFW 拦截")
                                    font: Style.fontCaption
                                    color: Style.textSecondary
                                    wrapMode: Text.Wrap
                                }

                                Switch {
                                    id: nsfwBlockIncomingSwitch
                                    checked: settingsViewModel ? settingsViewModel.nsfwBlockIncoming : false
                                    onToggled: if (settingsViewModel) settingsViewModel.nsfwBlockIncoming = checked
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: Style.spacingSmall

                                Label {
                                    text: qsTr("NSFW 阈值")
                                    font: Style.fontCaption
                                    color: Style.textSecondary
                                }

                                Slider {
                                    id: nsfwThresholdSlider
                                    Layout.fillWidth: true
                                    from: 0.0
                                    to: 1.0
                                    stepSize: 0.05
                                    value: settingsViewModel ? settingsViewModel.nsfwThreshold : 0.8
                                    onValueChanged: if (settingsViewModel && settingsViewModel.nsfwThreshold !== value) settingsViewModel.nsfwThreshold = value
                                }

                                Label {
                                    text: settingsViewModel ? (Math.round(settingsViewModel.nsfwThreshold * 100) + "%") : "80%"
                                    font: Style.fontCaption
                                    color: Style.textSecondary
                                    horizontalAlignment: Text.AlignRight
                                    Layout.preferredWidth: 56
                                }
                            }
                        }
                    }

                    // About section
                    GroupBox {
                        title: qsTr("关于")
                        Layout.fillWidth: true
                        leftPadding: Style.spacingLarge
                        rightPadding: Style.spacingLarge
                        topPadding: Style.spacingMedium
                        bottomPadding: Style.spacingMedium
                        background: Rectangle {
                            color: Style.surface
                            radius: 0
                            border.width: 1
                            border.color: "#E5E7EB"
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 0
                            spacing: Style.spacingSmall

                            // App name and version
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: Style.spacingSmall

                                Image {
                                    source: "../resources/app_icon.svg"
                                    sourceSize.width: 48
                                    sourceSize.height: 48
                                }

                                ColumnLayout {
                                    spacing: 2

                                    Label {
                                        text: "FlyKylin"
                                        font: Style.fontHeading
                                        color: Style.textPrimary
                                    }

                                    Label {
                                        text: qsTr("AI智能飞秋 - 局域网通讯工具")
                                        font: Style.fontCaption
                                        color: Style.textSecondary
                                    }
                                }
                            }

                            // Version info
                            GridLayout {
                                Layout.fillWidth: true
                                Layout.topMargin: Style.spacingSmall
                                columns: 2
                                columnSpacing: Style.spacingMedium
                                rowSpacing: 4

                                Label {
                                    text: qsTr("版本")
                                    font: Style.fontCaption
                                    color: Style.textSecondary
                                }
                                Label {
                                    text: settingsViewModel ? settingsViewModel.appVersion : "1.0.0"
                                    font: Style.fontCaption
                                    color: Style.textPrimary
                                }

                                Label {
                                    text: qsTr("构建日期")
                                    font: Style.fontCaption
                                    color: Style.textSecondary
                                }
                                Label {
                                    text: settingsViewModel ? settingsViewModel.buildDate : "-"
                                    font: Style.fontCaption
                                    color: Style.textPrimary
                                }

                                Label {
                                    text: qsTr("Qt 版本")
                                    font: Style.fontCaption
                                    color: Style.textSecondary
                                }
                                Label {
                                    text: settingsViewModel ? settingsViewModel.qtVersion : "-"
                                    font: Style.fontCaption
                                    color: Style.textPrimary
                                }

                                Label {
                                    text: qsTr("平台")
                                    font: Style.fontCaption
                                    color: Style.textSecondary
                                }
                                Label {
                                    text: settingsViewModel ? settingsViewModel.platform : "-"
                                    font: Style.fontCaption
                                    color: Style.textPrimary
                                }
                            }

                            // Features
                            Label {
                                text: qsTr("功能特性")
                                font: Style.fontCaption
                                color: Style.textSecondary
                                Layout.topMargin: Style.spacingSmall
                            }

                            Flow {
                                Layout.fillWidth: true
                                spacing: 6

                                Repeater {
                                    model: settingsViewModel ? settingsViewModel.features : []

                                    delegate: Rectangle {
                                        width: featureLabel.implicitWidth + 12
                                        height: featureLabel.implicitHeight + 6
                                        radius: 4
                                        color: "#EEF2FF"

                                        Label {
                                            id: featureLabel
                                            anchors.centerIn: parent
                                            text: modelData
                                            font: Style.fontCaption
                                            color: Style.primary
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item { Layout.preferredHeight: Style.spacingMedium }
                }
            }
        }
    }
}
