# FlyKylin 技术债务清单

**最后更新**: 2024-11-20  
**状态**: 活跃追踪

---

## 📊 债务概览

| 总计 | Critical | High | Medium | Low |
|------|----------|------|--------|-----|
| 10 | 0 | 5 | 3 | 2 |

**总预计工作量**: ~41小时

---

## 🔴 Critical级别（阻塞性）

无

---

## 🟠 High级别（重要）

### TD-001: 实现Protobuf序列化替换简单文本协议

**债务ID**: TD-001  
**来源**: Sprint 1 Code Review  
**影响范围**: `src/core/communication/PeerDiscovery.cpp`  
**优先级**: High  
**预计工作量**: 8小时

**描述**:  
当前UDP节点发现使用简单文本协议（`"FLYKYLIN|type|port|..."`）进行消息序列化。这是Sprint 1为快速验证功能的临时方案，存在以下问题：
- 缺乏类型安全
- 不支持向后兼容
- 手动序列化容易出错
- 性能不如二进制协议

**影响**:  
- 将来协议变更需要重构
- 不利于跨平台兼容性
- 未来消息扩展困难

**解决方案**:
1. 设计Protobuf消息结构（2h）
2. 实现Protobuf序列化/反序列化（4h）
3. 更新测试用例（2h）

**计划解决Sprint**: Sprint 2  
**负责人**: Development Execution Agent

**参考**:  
- 原始设计: `docs/requirements/backlog/US-001_UDP节点发现.md#Protobuf消息定义`
- Code Review: `docs/code-review/2024-11/sprint1-review.md#Suggestion-2`

---

## 🟡 Medium级别（建议改进）

### TD-002: 缓存网络接口列表避免重复枚举

**债务ID**: TD-002  
**来源**: Sprint 1 Code Review - Minor-1  
**影响范围**: `src/core/communication/PeerDiscovery.cpp:144-149`  
**优先级**: Medium  
**预计工作量**: 2小时

**描述**:  
```cpp
// 当前实现：每次接收数据包都枚举网络接口
QList<QHostAddress> localAddresses;
for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
    for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
        localAddresses.append(entry.ip());
    }
}
```

网络接口枚举是耗时操作，在高频率UDP数据包接收时可能影响性能。

**影响**:  
- 高频消息场景下可能出现性能瓶颈
- 不必要的CPU消耗

**解决方案**:
```cpp
// 优化：在start()时缓存本机地址列表
// 头文件添加成员变量
QList<QHostAddress> m_localAddresses;

// start()中初始化并缓存
void PeerDiscovery::start() {
    // ... 现有代码 ...
    
    // 缓存本机地址
    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
            m_localAddresses.append(entry.ip());
        }
    }
}

// onDatagramReceived()中直接使用
if (m_localAddresses.contains(senderAddress)) {
    continue;
}
```

**计划解决Sprint**: Sprint 2  
**负责人**: Development Execution Agent

---

### TD-003: UI Model增量更新优化

**债务ID**: TD-003  
**来源**: Sprint 1 Code Review - Minor-2  
**影响范围**: `src/ui/viewmodels/PeerListViewModel.cpp:70-73`  
**优先级**: Medium  
**预计工作量**: 4小时

**描述**:  
```cpp
// 当前实现：每次更新都清空重建整个模型
void PeerListViewModel::updateModel() {
    m_model->removeRows(0, m_model->rowCount());
    // 重新添加所有项...
}
```

大量节点时，频繁重建可能导致UI闪烁和性能下降。

**影响**:  
- 当前影响：低（节点数量小）
- 未来影响：中（节点数量增多时）

**解决方案**:  
实现增量更新：
1. 对于新增节点：使用`insertRow()`
2. 对于更新节点：使用`setData()`
3. 对于删除节点：使用`removeRow()`

**计划解决Sprint**: Sprint 3  
**负责人**: Development Execution Agent

---

### TD-004: 使用枚举替代魔法数字

**债务ID**: TD-004  
**来源**: Sprint 1 Code Review - Minor-3  
**影响范围**: `src/core/communication/PeerDiscovery.cpp:128, 167`  
**优先级**: Medium  
**预计工作量**: 2小时

**描述**:  
```cpp
// 硬编码的消息类型
void PeerDiscovery::onBroadcastTimer() {
    sendBroadcast(3); // MSG_HEARTBEAT = 3
}
```

使用魔法数字降低代码可读性。

**解决方案**:
```cpp
// 使用枚举替代魔法数字
enum class MessageType : int {
    Hello = 1,
    Goodbye = 2,
    Heartbeat = 3
};

void PeerDiscovery::onBroadcastTimer() {
    sendBroadcast(static_cast<int>(MessageType::Heartbeat));
}
```

**计划解决Sprint**: Sprint 2  
**负责人**: Development Execution Agent

---

### TD-005: 充分利用C++20特性

**债务ID**: TD-005  
**来源**: Sprint 1 Code Review - Minor-4  
**影响范围**: `src/ui/viewmodels/PeerListViewModel.cpp:76`  
**优先级**: Medium  
**预计工作量**: 3小时

**描述**:  
```cpp
// 当前代码
for (auto it = m_peers.constBegin(); it != m_peers.constEnd(); ++it) {
    const auto& peer = it.value();
    // ...
}
```

未充分利用C++20 Range-based for loop特性。

**解决方案**:
```cpp
// 使用C++20 Range-based for loop（更简洁）
for (const auto& [userId, peer] : m_peers.asKeyValueRange()) {
    // 直接使用peer，无需it.value()
    if (!matchesFilter(peer)) continue;
    // ...
}
```

**计划解决Sprint**: Sprint 3（代码重构时统一优化）  
**负责人**: Code Review Agent

---

## 🟢 Low级别（优化建议）

### TD-006: 添加日志级别控制

**债务ID**: TD-006  
**来源**: Sprint 1 Code Review - Suggestion-1  
**影响范围**: 全局  
**优先级**: Low  
**预计工作量**: 3小时

**描述**:  
当前代码中大量使用`qDebug()`输出，在Release模式下可能影响性能。

**解决方案**:
```cpp
// 使用Qt的日志分类
Q_LOGGING_CATEGORY(peerDiscovery, "flykylin.core.peerdiscovery")

// 使用
qCDebug(peerDiscovery) << "Debug message";  // 可配置关闭
qCInfo(peerDiscovery) << "Info message";
qCWarning(peerDiscovery) << "Warning message";
```

**计划解决Sprint**: Sprint 3  
**负责人**: Development Execution Agent

---

### TD-007: 添加UI层单元测试

**债务ID**: TD-007  
**来源**: Sprint 1 Code Review - Suggestion-3  
**影响范围**: `tests/ui/`  
**优先级**: Low  
**预计工作量**: 6小时

**描述**:  
当前仅测试Core层（PeerNode, PeerDiscovery），缺少UI层（ViewModel和Widget）的单元测试。

**解决方案**:
```cpp
// tests/ui/PeerListViewModel_test.cpp
TEST_F(PeerListViewModelTest, FilterKeyword_UpdatesModel) {
    PeerListViewModel viewModel;
    // 添加测试节点
    // 设置过滤关键词
    // 验证model行数
}
```

**计划解决Sprint**: Sprint 3  
**负责人**: Testing Agent

---

## 📋 债务管理流程

### 新增债务

1. 通过Code Review、Bug报告或回顾会议识别
2. 填写债务模板（ID、描述、影响、优先级）
3. 添加到本文档
4. 在Sprint规划时评估是否纳入

### 解决债务

1. 在Sprint规划时分配到相应Sprint
2. 创建具体任务并实施
3. 完成后从本文档移除
4. 记录到`docs/technical-debt-resolved.md`

### 债务审查

- **频率**: 每个Sprint回顾会议
- **负责人**: Quality Assurance Agent
- **动作**: 更新优先级、合并重复、清理已解决

---

## 📊 债务趋势

| Sprint | 新增 | 解决 | 累计 |
|--------|------|------|------|
| Sprint 1 | 7 | 0 | 7 |
| Sprint 2 | TBD | 计划3 | 预计4 |

**目标**: 每个Sprint解决至少2个债务，保持累计债务≤10个。

---

---

## 🟠 Sprint 2新增债务

### TD-008: Message类缺少status字段

**债务ID**: TD-008  
**来源**: Sprint 2实现  
**影响范围**: `src/core/models/Message.h`  
**优先级**: High  
**预计工作量**: 2小时

**描述**:  
当前Message类无法表示消息状态（发送中、已送达、已读、失败等）。MessageService中多处需要设置状态但被注释掉。

**解决方案**:
```cpp
// Message.h添加
enum class MessageStatus {
    Sending,
    Sent,
    Delivered,
    Read,
    Failed
};

class Message {
    // ...
    MessageStatus status() const;
    void setStatus(MessageStatus status);
private:
    MessageStatus m_status = MessageStatus::Sending;
};
```

**计划解决Sprint**: Sprint 3  
**负责人**: Development Execution Agent

---

### TD-009: UserProfile未实现单例模式

**债务ID**: TD-009  
**来源**: Sprint 2实现  
**影响范围**: `src/core/config/UserProfile.h`  
**优先级**: High  
**预计工作量**: 3小时

**描述**:  
UserProfile类存在但不是单例，导致MessageService等使用占位符"local_user"。

**解决方案**:
```cpp
class UserProfile {
public:
    static UserProfile& instance();
    QString userId() const;
    QString userName() const;
    // ...
private:
    UserProfile();
    static UserProfile* s_instance;
};
```

**计划解决Sprint**: Sprint 3  
**负责人**: Development Execution Agent

---

### TD-010: 缺少MessageService单元测试

**债务ID**: TD-010  
**来源**: Sprint 2 Code Review  
**影响范围**: `tests/core/services/`  
**优先级**: High  
**预计工作量**: 6小时

**描述**:  
MessageService是核心服务，但没有单元测试，降低代码质量保障。

**解决方案**:  
创建`MessageService_test.cpp`，测试：
- Protobuf序列化/反序列化
- 消息发送/接收
- 错误处理
- Echo Bot集成

**计划解决Sprint**: Sprint 3  
**负责人**: Testing Agent

---

### TD-011: 缺少ChatViewModel单元测试

**债务ID**: TD-011  
**来源**: Sprint 2 Code Review  
**影响范围**: `tests/ui/viewmodels/`  
**优先级**: High  
**预计工作量**: 4小时

**描述**:  
ChatViewModel是MVVM关键组件，但没有单元测试。

**解决方案**:  
测试sendMessage、messageReceived等核心流程

**计划解决Sprint**: Sprint 3  
**负责人**: Testing Agent

---

### TD-012: 消息仅存储在内存

**债务ID**: TD-012  
**来源**: Sprint 2实现  
**影响范围**: `src/core/services/MessageService.cpp`  
**优先级**: Medium  
**预计工作量**: 6小时

**描述**:  
消息历史只存储在QMap中，程序重启后丢失。

**解决方案**:  
使用SQLite持久化消息

**计划解决Sprint**: Sprint 4  
**负责人**: Development Execution Agent

---

## 🎯 Sprint 2债务清理结果

### 已解决
1. **TD-001** (High, 8h) - ✅ Protobuf序列化 - 已完成

### 部分解决
2. **TD-004** (Medium, 2h) - ⚠️ 枚举替代魔法数字 - Protobuf中已使用枚举

### 未解决
3. **TD-002** (Medium, 2h) - 缓存网络接口 - 延后至Sprint 3

### 新增债务
- TD-008: Message status字段
- TD-009: UserProfile单例
- TD-010: MessageService单元测试
- TD-011: ChatViewModel单元测试
- TD-012: 消息持久化

**Sprint 2债务变化**: 7个 → 10个（新增5个，解决1个，部分解决1个）

---

## 🎯 Sprint 3优先清理

根据优先级和工作量，Sprint 3计划清理以下债务：

1. **TD-008** (High, 2h) - Message status字段
2. **TD-009** (High, 3h) - UserProfile单例
3. **TD-010** (High, 6h) - MessageService单元测试
4. **TD-011** (High, 4h) - ChatViewModel单元测试
5. **TD-002** (Medium, 2h) - 缓存网络接口

**总计**: 17小时（Sprint 3预留3 SP用于技术债务清理）

---

**维护人**: Quality Assurance Agent  
**审查周期**: 每Sprint  
**文档版本**: 1.1  
**最后更新**: 2024-11-20 (Sprint 2结束)
