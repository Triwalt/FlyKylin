/**
 * FlyKylin 主窗口 (QML)
 * 
 * 这是现代化重构的QML UI示例，将在Sprint 3全面实施
 */

import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import "."

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1200
    height: 800
    title: qsTr("FlyKylin - AI智能飞秋")
    
    // 主背景
    color: Style.background
    
    // Load Material Icons
    FontLoader {
        id: materialIcons
        source: "../assets/fonts/MaterialIcons-Regular.ttf"
    }
    
    // 设置对话框
    SettingsDialog {
        id: settingsDialog
    }

    PeerInfoDialog {
        id: peerInfoDialog
    }

    // 简单联系人数据模型（会话中“加入联系人”时填充）
    ListModel {
        id: contactsModel
    }

    // 持久化联系人列表
    Settings {
        id: contactsStore
        category: "contacts"
        property string contactsJson: "[]"
    }

    // 群组数据模型（本地伪群聊）
    ListModel {
        id: groupsModel
    }

    // 持久化群组列表
    Settings {
        id: groupsStore
        category: "groups"
        property string groupsJson: "[]"
    }

    function normalizeGroupMembers(members) {
        var result = []
        if (!members)
            return result

        // Handle JavaScript Array
        if (members instanceof Array) {
            for (var i = 0; i < members.length; ++i) {
                result.push(members[i])
            }
            return result
        }

        // Handle QML ListModel internal representation (has count property)
        if (typeof members.count === 'number') {
            for (var i = 0; i < members.count; ++i) {
                var item = members.get(i)
                if (item && item.modelData !== undefined) {
                    result.push(item.modelData)
                } else if (item) {
                    // Try to get the value directly
                    result.push(item)
                }
            }
            if (result.length > 0)
                return result
        }

        // Handle object with numeric keys (fallback)
        var keys = Object.keys(members)
        for (var i = 0; i < keys.length; ++i) {
            var k = keys[i]
            if (k === "length" || k === "count" || k === "objectName")
                continue
            var idx = parseInt(k)
            if (isNaN(idx))
                continue
            if (members[k] !== undefined)
                result.push(members[k])
        }
        return result
    }

    function countGroupMembers(members) {
        var arr = normalizeGroupMembers(members)
        return arr.length
    }

    function loadContacts() {
        contactsModel.clear()
        if (!contactsStore.contactsJson || contactsStore.contactsJson.length === 0)
            return

        try {
            var arr = JSON.parse(contactsStore.contactsJson)
            for (var i = 0; i < arr.length; ++i) {
                var c = arr[i]
                if (!c.userId)
                    continue
                contactsModel.append({
                                         userId: c.userId,
                                         userName: c.userName || "",
                                         hostName: c.hostName || "",
                                         ipAddress: c.ipAddress || "",
                                         tcpPort: c.tcpPort || 0
                                     })

                if (peerListViewModel && peerListViewModel.upsertPeerFromContact) {
                    peerListViewModel.upsertPeerFromContact(
                                c.userId,
                                c.userName || "",
                                c.hostName || "",
                                c.ipAddress || "",
                                c.tcpPort || 0)
                }
            }
        } catch (e) {
            console.log("[Contacts] Failed to parse contactsJson", e)
        }
    }

    function saveContacts() {
        var arr = []
        for (var i = 0; i < contactsModel.count; ++i) {
            var c = contactsModel.get(i)
            arr.push({
                         userId: c.userId,
                         userName: c.userName,
                         hostName: c.hostName,
                         ipAddress: c.ipAddress,
                         tcpPort: c.tcpPort
                     })
        }
        contactsStore.contactsJson = JSON.stringify(arr)
    }

    function loadGroups() {
        groupsModel.clear()
        if (!groupsStore.groupsJson || groupsStore.groupsJson.length === 0)
            return

        try {
            var arr = JSON.parse(groupsStore.groupsJson)
            for (var i = 0; i < arr.length; ++i) {
                var g = arr[i]
                if (!g.id || !g.name)
                    continue
                // members can be stored as array or JSON string
                var membersArray = []
                if (typeof g.members === 'string') {
                    try {
                        membersArray = JSON.parse(g.members)
                    } catch (e) {
                        membersArray = []
                    }
                } else if (g.members) {
                    membersArray = normalizeGroupMembers(g.members)
                }
                if (settingsViewModel && settingsViewModel.localUserId
                        && membersArray.indexOf(settingsViewModel.localUserId) === -1) {
                    membersArray.push(settingsViewModel.localUserId)
                }
                // Store members as JSON string to avoid QML ListModel array conversion issues
                groupsModel.append({
                                       id: g.id,
                                       name: g.name,
                                       membersJson: JSON.stringify(membersArray),
                                       memberCount: membersArray.length
                                   })
                console.log("[GroupChat] loadGroups: loaded group", g.id, "members:", JSON.stringify(membersArray))
            }
        } catch (e) {
            console.log("[GroupChat] Failed to parse groupsJson", e)
        }
    }

    // Helper function to get members array from group model item
    function getGroupMembers(groupItem) {
        if (!groupItem)
            return []
        if (groupItem.membersJson) {
            try {
                return JSON.parse(groupItem.membersJson)
            } catch (e) {
                return []
            }
        }
        // Fallback for old format
        return normalizeGroupMembers(groupItem.members)
    }

    function saveGroups() {
        var arr = []
        for (var i = 0; i < groupsModel.count; ++i) {
            var g = groupsModel.get(i)
            var membersArray = getGroupMembers(g)
            console.log("[GroupChat] saveGroups: group", g.id, "members:", JSON.stringify(membersArray))
            arr.push({
                         id: g.id,
                         name: g.name,
                         members: JSON.stringify(membersArray),
                         memberCount: membersArray.length
                     })
        }
        groupsStore.groupsJson = JSON.stringify(arr)
        console.log("[GroupChat] saveGroups: saved", arr.length, "groups")
    }

    Component.onCompleted: {
        loadContacts()
        loadGroups()
    }

    // 简单的下线提醒横幅
    property string offlineNoticeText: ""
    property bool offlineNoticeVisible: false

    Timer {
        id: offlineNoticeTimer
        interval: 3000
        repeat: false
        onTriggered: offlineNoticeVisible = false
    }

    Rectangle {
        id: offlineNoticeBanner
        visible: offlineNoticeVisible
        radius: Style.radiusMedium
        color: "#323232CC"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 16
        z: 1000

        Label {
            id: offlineNoticeLabel
            anchors.margins: 12
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: offlineNoticeText
            font: Style.fontBody
            color: "white"
        }

        implicitWidth: offlineNoticeLabel.implicitWidth + 24
        implicitHeight: offlineNoticeLabel.implicitHeight + 16
    }

    // 主布局
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // 左侧：图标竖栏 + 列表
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: Style.sidebarWidth
            color: Style.surface
            
            // 右侧边框
            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: "#E5E7EB" // Border color
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 0
                spacing: 0

                Rectangle {
                    id: sideIconBar
                    Layout.fillHeight: true
                    Layout.preferredWidth: Style.iconBarWidth
                    color: Style.surface

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 0
                        spacing: 0

                        // 顶部应用 Logo（SVG，小尺寸，宽度为侧栏宽度的90%）
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 48
                            Layout.topMargin: Style.spacingMedium
                            color: "transparent"

                            Image {
                                anchors.centerIn: parent
                                width: parent.width * 0.9
                                source: "../resources/app_icon.svg"
                                fillMode: Image.PreserveAspectFit
                            }
                        }

                        // 四个图标 Tab（垂直排列）
                        ColumnLayout {
                            id: leftTabs
                            Layout.fillWidth: true
                            Layout.fillHeight: false
                            spacing: 4
                            property int currentIndex: 0

                            Item {
                                id: tabChatItem
                                Layout.fillWidth: true
                                Layout.preferredHeight: Style.tabHeight

                                Rectangle {
                                    id: tabChatBg
                                    anchors.fill: parent
                                    radius: Style.radiusMedium
                                    color: leftTabs.currentIndex === 0
                                           ? "#EEF2FF"
                                           : (chatMouseArea.containsMouse ? Style.surfaceHover : "transparent")
                                }

                                Text {
                                    anchors.centerIn: parent
                                    anchors.horizontalCenterOffset: 2
                                    text: "\ue0b7" // chat icon
                                    font: Style.iconFont
                                    color: leftTabs.currentIndex === 0 ? Style.primary : Style.textSecondary
                                }

                                MouseArea {
                                    id: chatMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: leftTabs.currentIndex = 0
                                }
                            }

                            // 在线
                            Item {
                                id: tabOnlineItem
                                Layout.fillWidth: true
                                Layout.preferredHeight: Style.tabHeight

                                Rectangle {
                                    id: tabOnlineBg
                                    anchors.fill: parent
                                    radius: Style.radiusMedium
                                    color: leftTabs.currentIndex === 1
                                           ? "#EEF2FF"
                                           : (onlineMouseArea.containsMouse ? Style.surfaceHover : "transparent")
                                }

                                Text {
                                    anchors.centerIn: parent
                                    anchors.horizontalCenterOffset: 2
                                    text: "\ue7f4" // people/online icon
                                    font: Style.iconFont
                                    color: leftTabs.currentIndex === 1 ? Style.primary : Style.textSecondary
                                }

                                MouseArea {
                                    id: onlineMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: leftTabs.currentIndex = 1
                                }
                            }

                            // 联系人
                            Item {
                                id: tabContactsItem
                                Layout.fillWidth: true
                                Layout.preferredHeight: Style.tabHeight

                                Rectangle {
                                    id: tabContactsBg
                                    anchors.fill: parent
                                    radius: Style.radiusMedium
                                    color: leftTabs.currentIndex === 2
                                           ? "#EEF2FF"
                                           : (contactsMouseArea.containsMouse ? Style.surfaceHover : "transparent")
                                }

                                Text {
                                    anchors.centerIn: parent
                                    anchors.horizontalCenterOffset: 2
                                    text: "\ue0ba" // contacts icon
                                    font: Style.iconFont
                                    color: leftTabs.currentIndex === 2 ? Style.primary : Style.textSecondary
                                }

                                MouseArea {
                                    id: contactsMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: leftTabs.currentIndex = 2
                                }
                            }

                            // 群组
                            Item {
                                id: tabGroupsItem
                                Layout.fillWidth: true
                                Layout.preferredHeight: 56

                                Rectangle {
                                    id: tabGroupsBg
                                    anchors.fill: parent
                                    radius: Style.radiusMedium
                                    color: leftTabs.currentIndex === 3
                                           ? "#EEF2FF"
                                           : (groupsMouseArea.containsMouse ? Style.surfaceHover : "transparent")
                                }

                                Text {
                                    anchors.centerIn: parent
                                    anchors.horizontalCenterOffset: 2
                                    text: "\ue7ef" // groups icon
                                    font: Style.iconFont
                                    color: leftTabs.currentIndex === 3 ? Style.primary : Style.textSecondary
                                }

                                MouseArea {
                                    id: groupsMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: leftTabs.currentIndex = 3
                                }
                            }
                        }

                        // 占位拉伸项，将设置按钮推到底部
                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }

                        // 底部设置按钮（样式与上方按钮类似）
                        Item {
                            id: settingsButtonItem
                            Layout.fillWidth: true
                            Layout.bottomMargin: Style.spacingMedium
                            Layout.preferredHeight: Style.tabHeight

                            Rectangle {
                                id: settingsBg
                                anchors.fill: parent
                                radius: Style.radiusMedium
                                color: settingsMouseArea.containsMouse ? Style.surfaceHover : "transparent"
                            }

                            Text {
                                anchors.centerIn: parent
                                anchors.horizontalCenterOffset: 2
                                text: "\ue8b8" // settings icon
                                font: Style.iconFont
                                color: Style.primary
                            }

                            MouseArea {
                                id: settingsMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: settingsDialog.visible = true
                            }
                        }
                    }
                }

                // 右侧：分页面内容
                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    spacing: 0

                    // 左侧分页面：会话 / 在线用户 / 联系人 / 群组
                    StackLayout {
                        id: leftStack
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        currentIndex: leftTabs.currentIndex

                        // 会话页：显示所有会话（包含离线用户）+ 本地群聊会话
                        Item {
                            id: sessionsPage

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: Style.spacingMedium
                                spacing: Style.spacingMedium

                                // 会话（用户）列表，沿用通用 PeerList 组件
                                PeerList {
                                    id: sessionPeerList
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    viewModel: peerListViewModel
                                    titleText: qsTr("会话")
                                    useSessionModel: true
                                }

                                // 群聊会话列表：展示所有本地定义的群组
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: Math.min(220, groupsModel.count * 56 + Style.spacingLarge)
                                    color: "transparent"

                                    ColumnLayout {
                                        anchors.fill: parent
                                        spacing: Style.spacingSmall

                                        Label {
                                            text: qsTr("群聊会话")
                                            font: Style.fontCaption
                                            color: Style.textSecondary
                                        }

                                        ListView {
                                            id: sessionGroupsList
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            model: groupsModel
                                            clip: true
                                            spacing: 4

                                            delegate: ItemDelegate {
                                                width: sessionGroupsList.width

                                                contentItem: ColumnLayout {
                                                    Layout.fillWidth: true
                                                    spacing: 2

                                                    Label {
                                                        text: name
                                                        font: Style.fontBody
                                                        color: Style.textPrimary
                                                    }

                                                    Label {
                                                        text: qsTr("成员数: %1").arg(memberCount)
                                                        font: Style.fontCaption
                                                        color: Style.textSecondary
                                                    }
                                                }

                                                onClicked: {
                                                    var m = membersJson ? JSON.parse(membersJson) : []
                                                    chatViewModel.setCurrentGroup(id, name, m)
                                                    // 保持在“会话”页，只切换右侧聊天为该群
                                                    leftTabs.currentIndex = 0
                                                }

                                                Menu {
                                                    id: sessionGroupContextMenu
                                                    MenuItem {
                                                        text: qsTr("删除群组")
                                                        onTriggered: {
                                                            var idx = index
                                                            if (idx >= 0 && idx < groupsModel.count) {
                                                                var g = groupsModel.get(idx)
                                                                groupsModel.remove(idx)
                                                                saveGroups()
                                                                if (chatViewModel.isGroupChat() && chatViewModel.getCurrentGroupId() === g.id) {
                                                                    chatViewModel.resetConversation()
                                                                }
                                                            }
                                                        }
                                                    }
                                                }

                                                MouseArea {
                                                    anchors.fill: parent
                                                    acceptedButtons: Qt.RightButton
                                                    onClicked: function(mouse) {
                                                        if (mouse.button === Qt.RightButton) {
                                                            sessionGroupContextMenu.open()
                                                            mouse.accepted = true
                                                        }
                                                    }
                                                }
                                            }

                                            ScrollBar.vertical: ScrollBar {}
                                        }
                                    }
                                }
                            }
                        }

                        // 在线用户页：仅显示当前在线用户
                        PeerList {
                            viewModel: peerListViewModel
                            titleText: qsTr("在线用户")
                            useSessionModel: false
                        }

                        // 联系人页
                        Item {
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: Style.spacingMedium
                                spacing: Style.spacingMedium

                                Label {
                                    text: qsTr("联系人")
                                    font: Style.fontHeading
                                    color: Style.textPrimary
                                }

                                ListView {
                                    id: contactsList
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    model: contactsModel
                                    clip: true
                                    spacing: 4

                                    delegate: ItemDelegate {
                                        width: contactsList.width

                                        contentItem: RowLayout {
                                            spacing: Style.spacingSmall

                                            // 在线状态指示灯
                                            Rectangle {
                                                width: 10
                                                height: 10
                                                radius: 5
                                                color: (peerListViewModel.onlineVersion,
                                                        peerListViewModel.isPeerOnline(userId)) ? Style.online : Style.offline
                                            }

                                            ColumnLayout {
                                                Layout.fillWidth: true
                                                spacing: 2

                                                Label {
                                                    text: userName
                                                    font: Style.fontBody
                                                    color: Style.textPrimary
                                                }

                                                Label {
                                                    text: ipAddress + ":" + (tcpPort > 0 ? tcpPort : "-")
                                                    font: Style.fontCaption
                                                    color: Style.textSecondary
                                                }
                                            }
                                        }

                                        Menu {
                                            id: contactContextMenu
                                            MenuItem {
                                                text: qsTr("删除联系人")
                                                onTriggered: {
                                                    if (index >= 0 && index < contactsModel.count) {
                                                        contactsModel.remove(index)
                                                        saveContacts()
                                                    }
                                                }
                                            }
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            acceptedButtons: Qt.RightButton
                                            onClicked: function(mouse) {
                                                if (mouse.button === Qt.RightButton) {
                                                    contactContextMenu.open()
                                                    mouse.accepted = true
                                                }
                                            }
                                        }
                                    }

                                    ScrollBar.vertical: ScrollBar {}
                                }
                            }
                        }

                        // 群组页（预留）
                        Item {
                            id: groupsPage

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: Style.spacingMedium
                                spacing: Style.spacingMedium

                                RowLayout {
                                    Layout.fillWidth: true

                                    Label {
                                        text: qsTr("群组")
                                        font: Style.fontHeading
                                        color: Style.textPrimary
                                        Layout.fillWidth: true
                                    }

                                    Button {
                                        text: qsTr("新建群组")
                                        onClicked: newGroupDialog.open()
                                    }
                                }

                                ListView {
                                    id: groupsList
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    model: groupsModel
                                    clip: true
                                    spacing: 4

                                    delegate: ItemDelegate {
                                        width: groupsList.width

                                        contentItem: ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2

                                            Label {
                                                text: name
                                                font: Style.fontBody
                                                color: Style.textPrimary
                                            }

                                            Label {
                                                text: qsTr("成员数: %1").arg(memberCount)
                                                font: Style.fontCaption
                                                color: Style.textSecondary
                                            }
                                        }

                                        onClicked: {
                                            var m = membersJson ? JSON.parse(membersJson) : []
                                            chatViewModel.setCurrentGroup(id, name, m)
                                            // 打开群聊后回到“会话”页，方便随时切换到其他会话
                                            leftTabs.currentIndex = 0
                                        }

                                        Menu {
                                            id: groupsPageContextMenu
                                            MenuItem {
                                                text: qsTr("删除群组")
                                                onTriggered: {
                                                    var idx = index
                                                    if (idx >= 0 && idx < groupsModel.count) {
                                                        var g = groupsModel.get(idx)
                                                        groupsModel.remove(idx)
                                                        saveGroups()
                                                        if (chatViewModel.isGroupChat() && chatViewModel.getCurrentGroupId() === g.id) {
                                                            chatViewModel.resetConversation()
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            acceptedButtons: Qt.RightButton
                                            onClicked: function(mouse) {
                                                if (mouse.button === Qt.RightButton) {
                                                    groupsPageContextMenu.open()
                                                    mouse.accepted = true
                                                }
                                            }
                                        }
                                    }

                                    ScrollBar.vertical: ScrollBar {}
                                }
                            }

                            Dialog {
                                id: newGroupDialog
                                modal: true
                                x: (mainWindow.width - width) / 2
                                y: (mainWindow.height - height) / 2
                                standardButtons: Dialog.Ok | Dialog.Cancel
                                title: qsTr("新建群组")
                                property var selectedIds: []

                                onOpened: {
                                    selectedIds = []
                                    groupNameField.text = ""
                                }

                                contentItem: ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: Style.spacingMedium
                                    spacing: Style.spacingSmall

                                    TextField {
                                        id: groupNameField
                                        Layout.fillWidth: true
                                        placeholderText: qsTr("群组名称")
                                    }

                                    Label {
                                        text: qsTr("从联系人中选择成员")
                                        font: Style.fontCaption
                                        color: Style.textSecondary
                                    }

                                    ListView {
                                        id: newGroupContactsList
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 260
                                        model: contactsModel
                                        clip: true

                                        delegate: ItemDelegate {
                                            width: newGroupContactsList.width

                                            contentItem: RowLayout {
                                                spacing: Style.spacingSmall

                                                CheckBox {
                                                    checked: newGroupDialog.selectedIds.indexOf(userId) !== -1
                                                    onToggled: {
                                                        var list = newGroupDialog.selectedIds.slice(0)
                                                        var idx = list.indexOf(userId)
                                                        if (checked && idx === -1)
                                                            list.push(userId)
                                                        else if (!checked && idx !== -1)
                                                            list.splice(idx, 1)
                                                        newGroupDialog.selectedIds = list
                                                    }
                                                }

                                                ColumnLayout {
                                                    Layout.fillWidth: true
                                                    spacing: 2

                                                    Label {
                                                        text: userName
                                                        font: Style.fontBody
                                                        color: Style.textPrimary
                                                    }

                                                    Label {
                                                        text: ipAddress + ":" + (tcpPort > 0 ? tcpPort : "-")
                                                        font: Style.fontCaption
                                                        color: Style.textSecondary
                                                    }
                                                }
                                            }
                                        }

                                        ScrollBar.vertical: ScrollBar {}

                                    }
                                }

                                onAccepted: {
                                    if (selectedIds.length === 0)
                                        return

                                    var groupId = "g-" + Date.now()
                                    var name = groupNameField.text && groupNameField.text.length > 0
                                               ? groupNameField.text
                                               : qsTr("新群组")
                                    var membersArray = normalizeGroupMembers(selectedIds)
                                    if (settingsViewModel && settingsViewModel.localUserId
                                            && membersArray.indexOf(settingsViewModel.localUserId) === -1) {
                                        membersArray.push(settingsViewModel.localUserId)
                                    }
                                    groupsModel.append({
                                                           id: groupId,
                                                           name: name,
                                                           membersJson: JSON.stringify(membersArray),
                                                           memberCount: membersArray.length
                                                       })
                                    saveGroups()
                                    if (chatViewModel && settingsViewModel && settingsViewModel.localUserId) {
                                        chatViewModel.registerGroup(groupId, membersArray, settingsViewModel.localUserId)
                                    }
                                    chatViewModel.setCurrentGroup(groupId, name, membersArray)
                                    // 新建群组后也回到“会话”页，左侧展示会话+群聊列表
                                    leftTabs.currentIndex = 0
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // 右侧：聊天区域
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: Style.background // Chat area background
            
            // 聊天视图
            ChatView {
                anchors.fill: parent
                viewModel: chatViewModel
                hostWindow: mainWindow
            }
        }
    }

    Dialog {
        id: addToGroupDialog
        modal: true
        x: (mainWindow.width - width) / 2
        y: (mainWindow.height - height) / 2
        standardButtons: Dialog.Ok | Dialog.Cancel
        title: qsTr("选择群组")
        property string pendingUserId: ""
        property string pendingUserName: ""

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: Style.spacingMedium
            spacing: Style.spacingSmall

            Label {
                text: groupsModel.count > 0
                      ? qsTr("为 %1 选择一个群组").arg(addToGroupDialog.pendingUserName)
                      : qsTr("当前还没有群组，请先在群组页中新建")
                font: Style.fontBody
                color: Style.textPrimary
                wrapMode: Text.Wrap
            }

            ComboBox {
                id: groupCombo
                Layout.fillWidth: true
                visible: groupsModel.count > 0
                model: groupsModel
                textRole: "name"
            }
        }

        onAccepted: {
            if (groupsModel.count === 0)
                return
            var index = groupCombo.currentIndex
            if (index < 0 || index >= groupsModel.count)
                return
            var g = groupsModel.get(index)
            var members = getGroupMembers(g)
            if (members.indexOf(pendingUserId) === -1) {
                members.push(pendingUserId)
                groupsModel.set(index, {
                                     id: g.id,
                                     name: g.name,
                                     membersJson: JSON.stringify(members),
                                     memberCount: members.length
                                 })
                saveGroups()

                if (chatViewModel && settingsViewModel && settingsViewModel.localUserId) {
                    chatViewModel.registerGroup(g.id, members, settingsViewModel.localUserId)
                }

                // Switch to the updated group chat and refresh aggregated history
                chatViewModel.setCurrentGroup(g.id, g.name, members)
                // 回到“会话”页，左侧展示会话+群聊列表，方便切换
                leftTabs.currentIndex = 0
            }
        }
    }

    // Auto-join groups when receiving group messages from other devices.
    Connections {
        target: chatViewModel

        onGroupMessageDiscovered: {
            console.log("[Main] onGroupMessageDiscovered:", groupId, fromUserId, toUserId)
            if (!groupId || groupId === "")
                return

            if (!settingsViewModel || !settingsViewModel.localUserId)
                return

            var localId = settingsViewModel.localUserId

            // Skip if this device is the sender; local groups are already
            // managed via groupsModel/saveGroups.
            if (fromUserId === localId)
                return

            // Check if the group already exists.
            for (var i = 0; i < groupsModel.count; ++i) {
                var g = groupsModel.get(i)
                if (g.id === groupId)
                    return
            }

            // Derive a minimal member list: local user + sender + receiver (if any).
            var members = []
            members.push(localId)
            if (fromUserId && members.indexOf(fromUserId) === -1)
                members.push(fromUserId)
            if (toUserId && members.indexOf(toUserId) === -1)
                members.push(toUserId)

            var name = qsTr("群组 %1").arg(groupId)
            console.log("[Main] Creating new group:", groupId, "members:", JSON.stringify(members))

            groupsModel.append({
                                   id: groupId,
                                   name: name,
                                   membersJson: JSON.stringify(members),
                                   memberCount: members.length
                               })
            saveGroups()

            if (chatViewModel) {
                chatViewModel.registerGroup(groupId, members, fromUserId)
            }
        }
    }

    Connections {
        target: peerListViewModel

        onPeerDetailsRequested: {
            peerInfoDialog.openWithArgs(userId, userName, hostName, ipAddress, tcpPort, groups)
        }

        onAddToContactsRequested: {
            console.log("[Main] onAddToContactsRequested received:", userId, userName, ipAddress, tcpPort)
            var displayName = (userName && userName.length > 0) ? userName : userId

            // 查找是否已存在
            var existingIndex = -1
            for (var i = 0; i < contactsModel.count; ++i) {
                var c = contactsModel.get(i)
                if (c.userId === userId) {
                    existingIndex = i
                    break
                }
            }

            var contactData = {
                userId: userId,
                userName: displayName,
                hostName: hostName,
                ipAddress: ipAddress,
                tcpPort: tcpPort
            }

            if (existingIndex === -1) {
                contactsModel.append(contactData)
            } else {
                contactsModel.set(existingIndex, contactData)
            }

            saveContacts()
            console.log("[Main] Contact saved, contactsModel.count:", contactsModel.count)

            if (peerListViewModel && peerListViewModel.upsertPeerFromContact) {
                peerListViewModel.upsertPeerFromContact(userId,
                                                        displayName,
                                                        hostName,
                                                        ipAddress,
                                                        tcpPort)
            }
        }

        onAddToGroupRequested: {
            if (groupsModel.count === 0) {
                offlineNoticeText = qsTr("当前还没有群组，请先在群组页中新建")
                offlineNoticeVisible = true
                offlineNoticeTimer.restart()
                return
            }

            addToGroupDialog.pendingUserId = userId
            addToGroupDialog.pendingUserName = (userName && userName.length > 0) ? userName : userId
            addToGroupDialog.open()
        }

        onPeerOfflineNotified: {
            var name = (userName && userName.length > 0) ? userName : userId
            offlineNoticeText = name + " (" + ipAddress + ") 已下线"
            offlineNoticeVisible = true
            offlineNoticeTimer.restart()
        }
    }
}
