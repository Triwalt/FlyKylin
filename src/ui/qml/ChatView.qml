/**
 * 聊天视图组件 (QML)
 */

/**
 * 聊天视图组件 (QML)
 */

import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3
import Qt.labs.settings 1.1
import "."

Item {
    id: chatView
    
    FontLoader {
        id: emojiFont
        source: "../assets/fonts/NotoColorEmoji.ttf"
    }

    property var viewModel
    property var hostWindow: null
    property bool hideWindowForScreenshot: true
    property bool screenshotSelectionVisible: false
    property string screenshotFullPath: ""
    property string pendingScrollMessageId: ""
    property string highlightMessageId: ""

    // 全局搜索排序模式：true 表示优先按相关性（语义重排），false 表示按时间
    property bool searchSortByRelevance: true

    // Emoji 分类数据（按类别分组，使用 Unicode + Noto Color Emoji 渲染）
    property var emojiCategories: [
        {
            key: "smileys",
            label: "😀",
            name: qsTr("表情"),
            emojis: "😀 😃 😄 😁 😆 😅 😂 🤣 😊 😉 😍 😘 😗 😙 😚 😜 🤔 😎 😏 😢 😭 😡 😱 😴 😇 😕 😮 😲 🤯 🤪 🥰 🤗 🤭 🤤".split(" ")
        },
        {
            key: "animals",
            label: "🐶",
            name: qsTr("动物"),
            emojis: "🐶 🐱 🐭 🐹 🐰 🦊 🐻 🐼 🐨 🐯 🦁 🐷 🐮 🐵 🙈 🙉 🙊 🐺 🐴 🐔 🐧 🐦 🐤 🐸 🐙 🐡 🐬 🐳 🐞 🐝 🐛".split(" ")
        },
        {
            key: "food",
            label: "🍎",
            name: qsTr("食物"),
            emojis: "🍎 🍊 🍋 🍌 🍉 🍇 🍓 🍒 🍑 🍍 🥝 🍅 🥕 🌽 🍔 🍕 🍟 🌭 🍣 🍜 🍱 🍚 🍛 🍜 🍣 🍩 🍪 🎂 🍰 🧁 🍫 🍿".split(" ")
        },
        {
            key: "activity",
            label: "⚽",
            name: qsTr("活动"),
            emojis: "⚽ 🏀 🏈 ⚾ 🎾 🏐 🏓 🏸 🥊 🏆 🎮 🎲 🎹 🎤 🎧 🎬 🎨 🎪 🎯 🎳 🧩 🎻 🎺".split(" ")
        },
        {
            key: "objects",
            label: "💡",
            name: qsTr("物品"),
            emojis: "💡 🔑 📱 💻 🖥 🖨 ⌨ 🖱 💾 📷 🎥 📺 🔋 🔌 📦 📌 📎 ✂️ 🛒 📚 🖊 🖋 🖌 📝 📅 📎 🔒 🔓".split(" ")
        }
    ]

    property int currentEmojiCategoryIndex: 0
    property bool customEmojiManageMode: false
    // Emoji/表情包 Tab 当前索引
    property int emojiTabIndex: 0

    // Emoji 托盘尺寸（可以按需微调）
    property int emojiPopupWidth: 420
    property int emojiPopupHeight: 320

    // 自定义图片表情列表（通过 Settings 持久化）
    ListModel {
        id: customEmojiModel
    }

    Settings {
        id: emojiSettings
        category: "emoji"
        property string customEmojiJson: "[]"
    }

    function loadCustomEmojis() {
        customEmojiModel.clear()
        if (!emojiSettings.customEmojiJson || emojiSettings.customEmojiJson.length === 0)
            return
        try {
            var arr = JSON.parse(emojiSettings.customEmojiJson)
            for (var i = 0; i < arr.length; ++i) {
                var e = arr[i]
                if (!e || !e.url)
                    continue
                customEmojiModel.append({
                                             url: e.url
                                         })
            }
        } catch (e) {
            console.log("[Emoji] Failed to parse customEmojiJson", e)
        }
    }

    // Global search results popup
    Popup {
        id: searchResultsPopup
        modal: false
        focus: true
        padding: 0
        width: 420
        height: 320
        parent: hostWindow ? hostWindow.contentItem : chatView
        x: chatView.width - width - 24
        y: 72
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            color: Style.surface
            radius: Style.radiusLarge
            border.width: 1
            border.color: "#E5E7EB"
        }

        onClosed: {
            if (globalSearchViewModel)
                globalSearchViewModel.clearResults()
        }

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: Style.spacingSmall
            spacing: Style.spacingSmall

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: qsTr("搜索结果")
                    font: Style.fontCaption
                    color: Style.textSecondary
                    Layout.fillWidth: true
                }

                RowLayout {
                    spacing: 4
                    visible: settingsViewModel ? settingsViewModel.semanticSearchEnabled : false

                    Label {
                        text: searchSortByRelevance ? qsTr("相关性") : qsTr("时间")
                        font: Style.fontCaption
                        color: Style.textSecondary
                    }

                    Switch {
                        id: sortModeSwitch
                        checked: searchSortByRelevance
                        onToggled: {
                            searchSortByRelevance = checked

                            if (!globalSearchViewModel)
                                return

                            var text = globalSearchField ? globalSearchField.text.trim() : ""
                            if (!text || text.length === 0)
                                return

                            var useSemantic = false
                            if (settingsViewModel && settingsViewModel.semanticSearchEnabled && chatView.searchSortByRelevance) {
                                useSemantic = true
                            }

                            globalSearchViewModel.search(text,
                                                         useSemantic,
                                                         "")
                            searchResultsPopup.open()
                        }
                    }
                }
            }

            ListView {
                id: searchResultsList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: globalSearchViewModel ? globalSearchViewModel.resultsModel : null
                spacing: 2

                delegate: ItemDelegate {
                    width: searchResultsList.width

                    contentItem: ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            text: model.content
                            font: Style.fontBody
                            color: Style.textPrimary
                            elide: Text.ElideRight
                        }

                        Label {
                            text: model.timestamp
                            font: Style.fontCaption
                            color: Style.textSecondary
                        }
                    }

                    onClicked: {
                        if (peerListViewModel && model.peerId) {
                            chatView.pendingScrollMessageId = model.messageId
                            peerListViewModel.selectPeer(model.peerId)
                        }
                        searchResultsPopup.close()
                    }
                }

                ScrollBar.vertical: ScrollBar {}
            }
        }
    }

    function saveCustomEmojis() {
        var arr = []
        for (var i = 0; i < customEmojiModel.count; ++i) {
            var it = customEmojiModel.get(i)
            arr.push({
                         url: it.url
                     })
        }
        emojiSettings.customEmojiJson = JSON.stringify(arr)
    }

    function addCustomEmoji(fileUrl) {
        if (!fileUrl || fileUrl === "")
            return
        if (viewModel && viewModel.isImageNsfw(fileUrl)) {
            console.log("[Emoji] NSFW image detected, skipping custom emoji:", fileUrl)
            return
        }
        customEmojiModel.append({ url: fileUrl })
        saveCustomEmojis()
    }

    function removeCustomEmoji(index) {
        if (index < 0 || index >= customEmojiModel.count)
            return
        customEmojiModel.remove(index)
        saveCustomEmojis()
    }

    function positionEmojiPopup() {
        if (!emojiBtn || !emojiPopup)
            return

        var popupWidth = emojiPopupWidth
        var popupHeight = emojiPopupHeight

        var pt = emojiBtn.mapToItem(null, 0, 0)
        var x = pt.x + (emojiBtn.width - popupWidth) / 2
        // 让托盘底边紧贴按钮上方一点
        var y = pt.y - popupHeight - 8

        if (hostWindow) {
            x = Math.max(0, Math.min(hostWindow.width - popupWidth, x))
        }
        console.log("[Emoji] btn:", pt.x, pt.y, "popup size:", popupWidth, popupHeight,
                    "final:", x, y)
        emojiPopup.x = x
        emojiPopup.y = y
    }

    Connections {
        target: viewModel
        function onMessagesUpdated() {
            if (!pendingScrollMessageId || pendingScrollMessageId === "")
                return
            if (!viewModel)
                return

            var targetId = pendingScrollMessageId

            Qt.callLater(function() {
                if (!viewModel)
                    return

                var row = viewModel.findMessageRow(targetId)
                if (row >= 0) {
                    messageList.positionViewAtIndex(row, ListView.Center)
                    messageList.currentIndex = row
                    highlightMessageId = targetId
                }

                if (pendingScrollMessageId === targetId)
                    pendingScrollMessageId = ""
            })
        }
    }

    Component.onCompleted: {
        loadCustomEmojis()
    }

    function startScreenshotCapture() {
        if (!viewModel)
            return

        // 关闭设置小面板，避免截屏时一直悬浮
        if (screenshotOptionsPopup.opened)
            screenshotOptionsPopup.close()

        if (hideWindowForScreenshot && hostWindow) {
            hostWindow.visible = false
            screenshotCaptureTimer.start()
        } else {
            startScreenshotCaptureInternal()
        }
    }

    function startScreenshotCaptureInternal() {
        if (!viewModel)
            return

        var path = viewModel.captureScreenForSelection()
        console.log("[ChatView] captureScreenForSelection returned:", path)

        if (hideWindowForScreenshot && hostWindow) {
            hostWindow.visible = true
        }

        if (!path || path === "")
            return

        screenshotFullPath = path
        screenshotSelectionVisible = true
    }

    function sendCroppedSelection() {
        if (!viewModel || !screenshotFullPath)
            return

        if (screenshotImage.status !== Image.Ready)
            return

        var imgW = screenshotImage.paintedWidth
        var imgH = screenshotImage.paintedHeight
        if (imgW <= 0 || imgH <= 0)
            return

        var imgLeft = (screenshotOverlay.width - imgW) / 2
        var imgTop = (screenshotOverlay.height - imgH) / 2

        var selLeft = Math.max(selectionRect.x, imgLeft)
        var selTop = Math.max(selectionRect.y, imgTop)
        var selRight = Math.min(selectionRect.x + selectionRect.width, imgLeft + imgW)
        var selBottom = Math.min(selectionRect.y + selectionRect.height, imgTop + imgH)

        var selWidth = selRight - selLeft
        var selHeight = selBottom - selTop
        if (selWidth <= 0 || selHeight <= 0)
            return

        var srcW = screenshotImage.sourceSize.width > 0 ? screenshotImage.sourceSize.width : imgW
        var srcH = screenshotImage.sourceSize.height > 0 ? screenshotImage.sourceSize.height : imgH

        var scaleX = srcW / imgW
        var scaleY = srcH / imgH

        var cropX = Math.round((selLeft - imgLeft) * scaleX)
        var cropY = Math.round((selTop - imgTop) * scaleY)
        var cropW = Math.round(selWidth * scaleX)
        var cropH = Math.round(selHeight * scaleY)

        console.log("[ChatView] sendCroppedSelection:", screenshotFullPath,
                    cropX, cropY, cropW, cropH)

        viewModel.sendCroppedScreenshot(screenshotFullPath, cropX, cropY, cropW, cropH)
        screenshotSelectionVisible = false
        screenshotFullPath = ""
    }

    Timer {
        id: screenshotCaptureTimer
        interval: 200
        repeat: false
        onTriggered: {
            startScreenshotCaptureInternal()
        }
    }
    
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

                Item {
                    Layout.fillWidth: true
                }

                // Global chat search box
                Rectangle {
                    id: globalSearchBox
                    Layout.preferredWidth: 260
                    Layout.alignment: Qt.AlignVCenter
                    height: 32
                    radius: 16
                    color: Style.background
                    border.width: 1
                    border.color: "#E5E7EB"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 6
                        spacing: 4

                        TextField {
                            id: globalSearchField
                            Layout.fillWidth: true
                            placeholderText: qsTr("搜索聊天记录")
                            font: Style.fontCaption
                            background: null
                            selectByMouse: true
                            onAccepted: {
                                if (!globalSearchViewModel)
                                    return

                                var useSemantic = false
                                if (settingsViewModel && settingsViewModel.semanticSearchEnabled && chatView.searchSortByRelevance) {
                                    useSemantic = true
                                }

                                globalSearchViewModel.search(text,
                                                            useSemantic,
                                                            "")
                                searchResultsPopup.open()
                            }
                        }

                        Button {
                            id: globalSearchButton
                            implicitWidth: 24
                            implicitHeight: 24
                            background: Rectangle {
                                anchors.fill: parent
                                radius: 12
                                color: "transparent"
                            }
                            contentItem: Label {
                                anchors.centerIn: parent
                                text: "\ue8b6" // search icon
                                font: Style.iconFont
                                color: Style.textSecondary
                            }
                            onClicked: globalSearchField.accepted()
                        }
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
                if (pendingScrollMessageId && pendingScrollMessageId !== "")
                    return
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
                        // NSFW 检测结果提示气泡（仅用于阻断占位，内容以 "[NSFW]" 开头且包含“阻断”）
                        property bool isNsfwInfo: isText && model.content && model.content.startsWith("[NSFW]")
                        property bool isNsfwBlocked: isNsfwInfo && model.content.indexOf("阻断") !== -1

                        // Width is at most 80% of available row width, with inner padding
                        width: Math.min(parent.width * 0.8, contentColumn.implicitWidth + 32)
                        height: contentColumn.implicitHeight + 24

                        anchors.right: model.isMine ? parent.right : undefined
                        anchors.left: model.isMine ? undefined : parent.left
                        anchors.rightMargin: Style.spacingMedium
                        anchors.leftMargin: Style.spacingMedium

                        color: msgContent.isNsfwInfo
                               ? "#FFFDE7"    // 浅黄色系统提示背景
                               : (model.isMine ? Style.primary : Style.surface)
                        radius: Style.radiusMedium

                        Rectangle {
                            id: highlightOverlay
                            anchors.fill: parent
                            radius: parent.radius
                            color: model.isMine ? "#40FFFFFF" : "#402196F3"
                            opacity: 0
                        }

                        SequentialAnimation {
                            id: highlightAnim
                            running: chatView.highlightMessageId === model.messageId
                            loops: 1

                            PropertyAnimation {
                                target: highlightOverlay
                                property: "opacity"
                                from: 0
                                to: 1
                                duration: 120
                            }

                            PropertyAnimation {
                                target: highlightOverlay
                                property: "opacity"
                                from: 1
                                to: 0
                                duration: 350
                            }
                        }

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
                                visible: !msgContent.isNsfwInfo

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
                                visible: msgContent.isText && !msgContent.isNsfwBlocked
                                width: parent.width
                                text: model.content
                                color: msgContent.isNsfwInfo
                                       ? "#F57F17"    // NSFW 提示文字颜色
                                       : (model.isMine ? Style.textOnPrimary : Style.textPrimary)
                                font: msgContent.isNsfwInfo ? Style.fontCaption : Style.fontBody
                                wrapMode: Text.Wrap
                            }

                            // NSFW 阻断占位图（近似等大的灰色块，中央感叹号 + Blocked 标识）
                            Rectangle {
                                id: nsfwBlockedPlaceholder
                                visible: msgContent.isNsfwBlocked
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: Math.min(260, parent.width)
                                height: width * 0.75    // 4:3 比例，接近常规图片气泡
                                radius: 8
                                color: "#EEEEEE"

                                Column {
                                    anchors.centerIn: parent
                                    width: parent.width
                                    spacing: 4

                                    Label {
                                        width: parent.width
                                        text: "!"
                                        font.pixelSize: 32
                                        color: "#757575"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    Label {
                                        width: parent.width
                                        text: qsTr("Blocked")
                                        font: Style.fontCaption
                                        color: "#424242"
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                }
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

                        // Shadow for received messages only（Qt5 版本暂时不启用阴影特效）
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

                    // NSFW pass indicator for images: green check + Pass under the bubble
                    Label {
                        id: nsfwPassLabel
                        visible: model.kind === "image" && model.nsfwPassed
                        anchors.verticalCenter: timestampLabel.verticalCenter
                        anchors.left: model.isMine ? undefined : timestampLabel.right
                        anchors.leftMargin: model.isMine ? 0 : 6
                        anchors.right: model.isMine ? timestampLabel.left : undefined
                        anchors.rightMargin: model.isMine ? 6 : 0
                        text: "\u2713 " + qsTr("Pass")
                        font: Style.fontCaption
                        color: "#2E7D32"
                        opacity: 0.9
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

                // Emoji 按钮
                Button {
                    id: emojiBtn
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    hoverEnabled: true

                    background: Rectangle {
                        color: parent.enabled ? Style.surface : Style.background
                        radius: 20

                        Label {
                            anchors.centerIn: parent
                            visible: !emojiBtn.hovered
                            text: "\ue24e" // emoji icon
                            font: Style.iconFont
                            color: Style.textSecondary
                        }

                        Label {
                            anchors.centerIn: parent
                            visible: emojiBtn.hovered
                            text: qsTr("表情")
                            font: Style.fontBody
                            color: Style.textSecondary
                        }
                    }

                    enabled: viewModel && viewModel.currentPeerName !== ""
                    onClicked: {
                        if (screenshotOptionsPopup.opened)
                            screenshotOptionsPopup.close()

                        if (emojiPopup.opened) {
                            emojiPopup.close()
                        } else {
                            chatView.positionEmojiPopup()
                            emojiPopup.open()
                        }
                    }
                }

                Button {
                    id: screenshotBtn
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    hoverEnabled: true

                    background: Rectangle {
                        color: parent.enabled ? Style.surface : Style.background
                        radius: 20

                        Label {
                            anchors.centerIn: parent
                            visible: !screenshotBtn.hovered && !screenshotOptionsPopup.opened
                            text: "\ue3c9" // crop/screenshot icon
                            font: Style.iconFont
                            color: Style.textSecondary
                        }

                        Label {
                            anchors.centerIn: parent
                            visible: screenshotBtn.hovered || screenshotOptionsPopup.opened
                            text: qsTr("截图")
                            font: Style.fontBody
                            color: Style.textSecondary
                        }
                    }

                    enabled: viewModel && viewModel.currentPeerName !== ""
                    onClicked: {
                        chatView.startScreenshotCapture()
                    }

                    onHoveredChanged: {
                        if (hovered) {
                            screenshotOptionsPopup.open()
                        }
                    }
                }

                // Image send button
                Button {
                    id: imageBtn
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    hoverEnabled: true

                    background: Rectangle {
                        color: parent.enabled ? Style.surface : Style.background
                        radius: 20

                        Label {
                            anchors.centerIn: parent
                            visible: !imageBtn.hovered
                            text: "\ue3f4" // image icon
                            font: Style.iconFont
                            color: Style.textSecondary
                        }

                        Label {
                            anchors.centerIn: parent
                            visible: imageBtn.hovered
                            text: qsTr("图片")
                            font: Style.fontBody
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
                    hoverEnabled: true

                    background: Rectangle {
                        color: parent.enabled ? Style.surface : Style.background
                        radius: 20

                        Label {
                            anchors.centerIn: parent
                            visible: !fileBtn.hovered
                            text: "\ue2c4" // attach file icon
                            font: Style.iconFont
                            color: Style.textSecondary
                        }

                        Label {
                            anchors.centerIn: parent
                            visible: fileBtn.hovered
                            text: qsTr("文件")
                            font: Style.fontBody
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

    // 自定义表情图片添加对话框
    FileDialog {
        id: customEmojiFileDialog
        title: qsTr("选择表情图片")
        nameFilters: [qsTr("Images (*.png *.jpg *.jpeg *.bmp *.gif)")]
        onAccepted: {
            if (selectedFile !== "")
                addCustomEmoji(selectedFile)
        }
    }

    // Emoji 与自定义表情托盘
    Popup {
        id: emojiPopup
        modal: false
        focus: true
        padding: Style.spacingSmall
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        // 实际宽高由属性控制，方便后续微调
        width: emojiPopupWidth
        height: emojiPopupHeight
        // 以窗口内容区为父项，保证与按钮同一坐标系
        parent: hostWindow ? hostWindow.contentItem : chatView
        background: Rectangle {
            color: Style.surface
            radius: Style.radiusLarge
            border.width: 1
            border.color: "#E5E7EB"
        }
        onOpened: chatView.positionEmojiPopup()

        contentItem: ColumnLayout {
            anchors.fill: parent
            spacing: Style.spacingSmall

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: Style.spacingSmall
                Layout.rightMargin: Style.spacingSmall
                Layout.preferredHeight: 40

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Style.spacingSmall

                    Button {
                        id: emojiTabButton
                        text: qsTr("Emoji")
                        font: Style.fontBody
                        checkable: true
                        checked: emojiTabIndex === 0
                        background: Rectangle {
                            radius: 12
                            color: emojiTabButton.checked ? Style.surfaceHover : "transparent"
                            border.width: emojiTabButton.checked ? 1 : 0
                            border.color: emojiTabButton.checked ? Style.primaryLight : "transparent"
                        }
                        contentItem: Text {
                            text: emojiTabButton.text
                            font: emojiTabButton.font
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: emojiTabButton.checked ? Style.textPrimary : Style.textSecondary
                        }
                        onClicked: emojiTabIndex = 0
                    }

                    Button {
                        id: stickerTabButton
                        text: qsTr("表情包")
                        font: Style.fontBody
                        checkable: true
                        checked: emojiTabIndex === 1
                        background: Rectangle {
                            radius: 12
                            color: stickerTabButton.checked ? Style.surfaceHover : "transparent"
                            border.width: stickerTabButton.checked ? 1 : 0
                            border.color: stickerTabButton.checked ? Style.primaryLight : "transparent"
                        }
                        contentItem: Text {
                            text: stickerTabButton.text
                            font: stickerTabButton.font
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: stickerTabButton.checked ? Style.textPrimary : Style.textSecondary
                        }
                        onClicked: emojiTabIndex = 1
                    }
                }

                Button {
                    id: manageEmojiBtn
                    visible: emojiTabIndex === 1
                    text: customEmojiManageMode ? qsTr("完成") : qsTr("管理")
                    font: Style.fontCaption
                    padding: 8
                    background: Rectangle {
                        radius: 12
                        color: manageEmojiBtn.down ? Style.primaryLight
                                                   : (manageEmojiBtn.hovered ? Style.surfaceHover : "transparent")
                        border.width: 1
                        border.color: manageEmojiBtn.down || manageEmojiBtn.hovered ? "#E5E7EB" : "transparent"
                    }
                    onClicked: customEmojiManageMode = !customEmojiManageMode
                }
            }

            StackLayout {
                id: emojiStack
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: emojiTabIndex

                // 内置 Emoji
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: Style.spacingSmall

                        RowLayout {
                            id: emojiCategoryRow
                            Layout.fillWidth: true
                            Layout.leftMargin: Style.spacingSmall
                            Layout.rightMargin: Style.spacingSmall
                            spacing: Style.spacingSmall

                            Repeater {
                                model: emojiCategories.length
                                Button {
                                    id: emojiCategoryButton
                                    text: emojiCategories[index].label
                                    font.family: emojiFont.name !== "" ? emojiFont.name : "Noto Color Emoji"
                                    checkable: true
                                    checked: currentEmojiCategoryIndex === index
                                    Layout.preferredWidth: 48
                                    Layout.preferredHeight: 48
                                    font.pixelSize: 26

                                    background: Rectangle {
                                        radius: height / 2
                                        color: emojiCategoryButton.checked ? Style.primaryLight : "transparent"
                                        border.width: 1
                                        border.color: emojiCategoryButton.checked ? Style.primary : "#E5E7EB"
                                    }
                                    contentItem: Text {
                                        text: emojiCategoryButton.text
                                        font: emojiCategoryButton.font
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        color: Style.textPrimary
                                    }
                                    onClicked: currentEmojiCategoryIndex = index
                                }
                            }
                        }

                        GridView {
                            id: emojiGrid
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.leftMargin: Style.spacingSmall
                            Layout.rightMargin: Style.spacingSmall
                            Layout.bottomMargin: Style.spacingSmall
                            clip: true
                            cellWidth: 56
                            cellHeight: 56

                            model: emojiCategories[currentEmojiCategoryIndex].emojis

                            delegate: Button {
                                id: emojiCell
                                text: modelData
                                font.pixelSize: 32
                                font.family: emojiFont.name !== "" ? emojiFont.name : "Noto Color Emoji"
                                hoverEnabled: true
                                padding: 0

                                background: Rectangle {
                                    radius: 10
                                    color: emojiCell.down || emojiCell.hovered ? "#EEF2FF" : "transparent"
                                }
                                onClicked: {
                                    if (inputField && viewModel) {
                                        var pos = inputField.cursorPosition
                                        var before = inputField.text.slice(0, pos)
                                        var after = inputField.text.slice(pos)
                                        inputField.text = before + modelData + after
                                        inputField.cursorPosition = pos + modelData.length
                                    }
                                    emojiPopup.close()
                                }
                            }
                        }
                    }
                }

                // 自定义图片表情
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: Style.spacingSmall

                        GridView {
                            id: customEmojiGrid
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.leftMargin: Style.spacingSmall
                            Layout.rightMargin: Style.spacingSmall
                            Layout.bottomMargin: Style.spacingSmall
                            clip: true
                            cellWidth: 56
                            cellHeight: 56
                            model: customEmojiModel

                            delegate: Item {
                                width: customEmojiGrid.cellWidth
                                height: customEmojiGrid.cellHeight

                                property string imageUrl: url

                                Rectangle {
                                    anchors.fill: parent
                                    radius: 8
                                    color: "transparent"

                                    Image {
                                        anchors.fill: parent
                                        anchors.margins: 4
                                        source: imageUrl
                                        fillMode: Image.PreserveAspectFit
                                    }
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    color: "#80000000"
                                    visible: customEmojiManageMode

                                    Text {
                                        anchors.centerIn: parent
                                        text: qsTr("删除")
                                        color: "white"
                                        font.pixelSize: 12
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        if (!viewModel)
                                            return

                                        if (customEmojiManageMode) {
                                            removeCustomEmoji(index)
                                        } else if (imageUrl !== "") {
                                            viewModel.sendImage(imageUrl)
                                            emojiPopup.close()
                                        }
                                    }
                                }
                            }

                            footer: Item {
                                width: customEmojiGrid.cellWidth
                                height: customEmojiGrid.cellHeight
                                visible: customEmojiManageMode

                                Rectangle {
                                    anchors.fill: parent
                                    radius: 8
                                    color: Style.background
                                    border.color: "#D1D5DB"

                                    Text {
                                        anchors.centerIn: parent
                                        text: "+"
                                        color: Style.textSecondary
                                        font.pixelSize: 20
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: customEmojiFileDialog.open()
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: screenshotOptionsPopup
        modal: false
        focus: false
        padding: Style.spacingSmall
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        x: screenshotBtn ? screenshotBtn.mapToItem(chatView, screenshotBtn.width, 0).x : 0
        y: screenshotBtn ? screenshotBtn.mapToItem(chatView, screenshotBtn.width, 0).y : 0
        contentItem: Item {
            id: screenshotOptionsRoot

            implicitWidth: screenshotOptionsRow.implicitWidth
            implicitHeight: screenshotOptionsRow.implicitHeight

            RowLayout {
                id: screenshotOptionsRow
                anchors.fill: parent
                spacing: Style.spacingSmall

                CheckBox {
                    id: hideWindowCheckBox
                    checked: hideWindowForScreenshot
                    onToggled: hideWindowForScreenshot = checked
                }

                Label {
                    text: qsTr("截屏时隐藏窗口")
                    font: Style.fontCaption
                    color: Style.textPrimary
                }
            }

            // 鼠标离开该小面板区域时自动关闭
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
                onExited: {
                    if (!screenshotBtn.hovered)
                        screenshotOptionsPopup.close()
                }
            }
        }
    }

    Rectangle {
        id: screenshotOverlay
        visible: screenshotSelectionVisible
        anchors.fill: parent
        color: "#80000000"
        z: 1000

        Image {
            id: screenshotImage
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            asynchronous: true
            cache: false
            source: screenshotFullPath !== "" ? ("file:///" + screenshotFullPath) : ""
        }

        Rectangle {
            id: selectionRect
            visible: selectionMouse.dragging
            border.color: Style.primary
            border.width: 2
            color: "#4088BFFF"
        }

        MouseArea {
            id: selectionMouse
            anchors.fill: parent
            enabled: screenshotSelectionVisible
            hoverEnabled: true
            property bool dragging: false
            property real startX: 0
            property real startY: 0

            onPressed: function(mouse) {
                dragging = true
                startX = mouse.x
                startY = mouse.y
                selectionRect.x = startX
                selectionRect.y = startY
                selectionRect.width = 0
                selectionRect.height = 0
            }

            onPositionChanged: function(mouse) {
                if (!dragging)
                    return

                var x1 = Math.min(startX, mouse.x)
                var y1 = Math.min(startY, mouse.y)
                var x2 = Math.max(startX, mouse.x)
                var y2 = Math.max(startY, mouse.y)

                selectionRect.x = x1
                selectionRect.y = y1
                selectionRect.width = x2 - x1
                selectionRect.height = y2 - y1
            }

            onReleased: function(mouse) {
                dragging = false
            }
        }

        RowLayout {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: Style.spacingMedium
            spacing: Style.spacingSmall

            Button {
                text: qsTr("取消")
                onClicked: {
                    screenshotSelectionVisible = false
                    screenshotFullPath = ""
                }
            }

            Button {
                text: qsTr("发送截图")
                enabled: selectionRect.width > 4 && selectionRect.height > 4
                onClicked: {
                    sendCroppedSelection()
                }
            }
        }
    }
}
