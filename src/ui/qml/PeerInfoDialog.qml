import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

Dialog {
    id: root
    modal: true
    standardButtons: Dialog.Close
    title: qsTr("用户信息")

    width: 320

    property string userId: ""
    property string userName: ""
    property string hostName: ""
    property string ipAddress: ""
    property int tcpPort: 0
    property var groups: []

    x: (parent ? (parent.width - width) / 2 : 0)
    y: (parent ? (parent.height - height) / 2 : 0)

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: Style.spacingMedium
        spacing: Style.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("用户ID")
                font: Style.fontBody
                color: Style.textSecondary
                Layout.preferredWidth: 64
            }
            Label {
                text: root.userId
                font: Style.fontBody
                color: Style.textPrimary
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("UUID")
                font: Style.fontBody
                color: Style.textSecondary
                Layout.preferredWidth: 64
            }
            Label {
                text: root.userId.split(":")[0]
                font: Style.fontBody
                color: Style.textPrimary
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("名称")
                font: Style.fontBody
                color: Style.textSecondary
                Layout.preferredWidth: 64
            }
            Label {
                text: root.userName
                font: Style.fontBody
                color: Style.textPrimary
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("IP 地址")
                font: Style.fontBody
                color: Style.textSecondary
                Layout.preferredWidth: 64
            }
            Label {
                text: root.ipAddress
                font: Style.fontBody
                color: Style.textPrimary
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("端口")
                font: Style.fontBody
                color: Style.textSecondary
                Layout.preferredWidth: 64
            }
            Label {
                text: root.tcpPort
                font: Style.fontBody
                color: Style.textPrimary
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("主机名")
                font: Style.fontBody
                color: Style.textSecondary
                Layout.preferredWidth: 64
            }
            Label {
                text: root.hostName
                font: Style.fontBody
                color: Style.textPrimary
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("群组")
                font: Style.fontBody
                color: Style.textSecondary
                Layout.preferredWidth: 64
            }
            Label {
                text: (root.groups && root.groups.length > 0) ? root.groups.join(", ") : qsTr("未加入任何群组")
                font: Style.fontBody
                color: Style.textPrimary
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
        }
    }

    function openWithArgs(id, name, host, ip, port, groupList) {
        userId = id
        userName = name
        hostName = host
        ipAddress = ip
        tcpPort = port
        groups = groupList || []
        open()
    }
}
