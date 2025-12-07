# 聊天滚动优化技术文档

## 概述

本文档记录了 FlyKylin 聊天应用中消息列表滚动行为的优化过程，包括气泡宽度自适应、滚动位置保持、回到底部按钮等功能的实现。

## 问题与解决方案

### 1. 气泡宽度自适应

**问题**: 消息气泡宽度固定，不随消息内容长度自适应。

**解决方案** (`src/ui/qml/ChatView.qml`):
```qml
Rectangle {
    id: msgContent
    property real maxBubbleWidth: bubbleItem.width * 0.70
    property real textWidth: msgText.visible ? Math.min(msgText.implicitWidth, maxBubbleWidth - 24) : 0
    property real imageWidth: msgImage.visible && msgImage.status === Image.Ready 
                              ? Math.min(msgImage.paintedWidth, maxBubbleWidth - 24) : 0
    
    // 图片消息使用图片实际宽度，其他消息使用内容宽度
    width: msgContent.isImage 
           ? Math.max(80, Math.min(maxBubbleWidth, imageWidth + 24)) 
           : Math.max(80, Math.min(maxBubbleWidth, actualContentWidth + 24))
}
```

**关键点**:
- 使用 `implicitWidth` 获取文本自然宽度
- 图片使用 `paintedWidth`（实际渲染宽度），需要等待 `status === Image.Ready`
- 设置最小宽度 80px，最大宽度为容器的 70%

### 2. 滚动位置保持（核心难点）

**问题**: 用户向上滚动查看历史消息时，收到新消息会自动滚动到底部，打断用户阅读。

**调试过程**:

1. **初始尝试**: 使用 `atYEnd` 属性判断是否在底部
   - 问题: `atYEnd` 在某些边界情况下不准确

2. **手动跟踪状态**: 添加 `userScrolledUp` 标志
   - 问题: `onContentYChanged` 在模型重建时会被触发，导致状态被错误重置

3. **监听 `onRowsAboutToBeInserted`**: 在新行插入前保存状态
   - 问题: `rebuildMessageModel()` 会先 `clear()` 再插入，此时 count=0，`isAtBottom` 计算为 true

4. **监听 `modelAboutToBeReset`**: 在模型重置前保存状态
   - 问题: QML 的 Connections 可能没有正确连接到 QStandardItemModel 的信号

5. **最终方案**: 在 C++ 端添加自定义信号 `aboutToRebuildModel`

**最终实现**:

C++ 端 (`src/ui/viewmodels/ChatViewModel.h`):
```cpp
signals:
    void aboutToRebuildModel();  // 在模型重建前发出
```

C++ 端 (`src/ui/viewmodels/ChatViewModel.cpp`):
```cpp
void ChatViewModel::rebuildMessageModel() {
    emit aboutToRebuildModel();  // 在 clear() 之前发出信号
    m_messageModel->clear();
    // ...
}
```

QML 端:
```qml
ListView {
    property bool wasAtBottomBeforeReset: true
    property real savedContentY: 0
    
    // 监听完整重建
    Connections {
        target: viewModel
        function onAboutToRebuildModel() {
            if (messageList.count > 0) {
                messageList.wasAtBottomBeforeReset = messageList.isAtBottom
                messageList.savedContentY = messageList.contentY
            }
        }
    }
    
    // 监听增量添加
    Connections {
        target: messageList.model
        function onRowsAboutToBeInserted(parent, first, last) {
            if (messageList.count > 0) {
                messageList.wasAtBottomBeforeReset = messageList.isAtBottom
                messageList.savedContentY = messageList.contentY
            }
        }
    }
    
    onCountChanged: {
        if (count === 0) return  // 忽略模型清空
        
        if (wasAtBottomBeforeReset) {
            Qt.callLater(function() { positionViewAtEnd() })
        } else {
            Qt.callLater(function() { contentY = savedContentY })
        }
    }
}
```

**关键洞察**:
- 保存 `contentY` 而非距离底部的距离，因为新消息添加到末尾，之前消息的位置不变
- 需要同时监听 `aboutToRebuildModel`（完整重建）和 `rowsAboutToBeInserted`（增量添加）

### 3. 消息闪烁优化

**问题**: 每次收到消息都会重建整个模型，导致列表闪烁。

**解决方案**: 增量添加消息

```cpp
// 新增方法：只添加单条消息
void ChatViewModel::appendMessageToModel(const core::Message& msg) {
    auto* item = new QStandardItem();
    // 设置消息数据...
    m_messageModel->appendRow(item);
}

// 单聊消息接收：增量添加
void ChatViewModel::onMessageReceived(const Message& message) {
    m_messages.append(message);
    appendMessageToModel(message);  // 不调用 rebuildMessageModel()
    emit messagesUpdated();
}
```

**适用场景**:
- 单聊新消息接收：增量添加
- 单聊新消息发送：增量添加
- 群组消息：需要排序，仍使用完整重建
- 消息状态更新：需要更新现有项，使用完整重建

### 4. 回到底部按钮

```qml
Rectangle {
    id: scrollToBottomBtn
    visible: !messageList.isAtBottom && messageList.count > 0
    
    property bool isAtBottom: distanceFromBottom < 100
    property real distanceFromBottom: Math.max(0, contentHeight - height - contentY)
    
    MouseArea {
        onClicked: messageList.positionViewAtEnd()
    }
}
```

### 5. 滚轮滚动优化

```qml
MouseArea {
    anchors.fill: parent
    acceptedButtons: Qt.NoButton
    onWheel: function(wheel) {
        // 使用 flick 实现平滑滚动，保留边界回弹效果
        messageList.flick(0, wheel.angleDelta.y * 8)
        wheel.accepted = true
    }
}
```

## 调试技巧

1. **添加日志**: 在关键位置添加 `console.log()` 输出状态
2. **检查信号触发顺序**: 模型操作的信号顺序很重要
3. **理解 QML 属性绑定**: 计算属性在依赖变化时会重新计算
4. **Qt.callLater()**: 延迟执行确保在布局完成后操作

## 文件修改清单

- `src/ui/qml/ChatView.qml` - 滚动逻辑、气泡宽度、回到底部按钮
- `src/ui/viewmodels/ChatViewModel.h` - 添加 `aboutToRebuildModel` 信号和 `appendMessageToModel` 方法声明
- `src/ui/viewmodels/ChatViewModel.cpp` - 实现增量添加和信号发射

## 测试场景

1. ✅ 在底部发送消息 → 保持在底部
2. ✅ 在底部接收消息 → 自动滚动到最新
3. ✅ 向上滚动后发送消息 → 保持当前位置
4. ✅ 向上滚动后接收消息 → 保持当前位置，显示回到底部按钮
5. ✅ 点击回到底部按钮 → 滚动到最新消息
6. ✅ 滚轮滚动 → 平滑滚动，边界有回弹
7. ✅ 消息更新无闪烁（单聊场景）
