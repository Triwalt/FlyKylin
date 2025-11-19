/**
 * 聊天视图组件 (QML)
 * 
 * 待Sprint 3实现
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: chatView
    
    // 占位符
    Label {
        anchors.centerIn: parent
        text: qsTr("选择一个用户开始聊天\n\n(Sprint 3将实现完整功能)")
        horizontalAlignment: Text.AlignHCenter
        color: "#999999"
        font.pixelSize: 16
    }
}
