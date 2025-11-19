# UDP vs TCP 文本聊天方案对比

## 背景

用户询问：是否应该用UDP广播（类似IPMsg）实现文本聊天？

## 方案对比

### 方案A：纯UDP广播（IPMsg式）

**实现示例**：
```cpp
// IPMsg风格的UDP消息发送
void sendUdpTextMessage(QString toUser, QString content) {
    QByteArray packet;
    packet.append(QString("FLYKYLIN_MSG|%1|%2|%3|%4")
        .arg(QDateTime::currentMSecsSinceEpoch())  // 消息ID
        .arg(m_userId)                              // 发送者
        .arg(toUser)                                // 接收者
        .arg(content)                               // 内容
        .toUtf8());
    
    m_socket->writeDatagram(packet, QHostAddress::Broadcast, kUdpPort);
}
```

**优点**：
- ✅ 实现简单（100行代码即可）
- ✅ 无需维护连接状态
- ✅ 群发消息效率高
- ✅ 延迟低（无三次握手）

**缺点**：
- ❌ **消息丢失无法察觉**：UDP不保证送达
- ❌ **乱序到达**：多条消息可能乱序
- ❌ **重复接收**：同一消息可能收到多次
- ❌ **需要自己实现确认机制**（ACK）
- ❌ **文件传输困难**：需要手动分片、重传、拼接
- ❌ **拥塞控制缺失**：可能导致网络风暴

**需要额外实现的功能**：
1. 消息确认（ACK）机制
2. 重传机制（超时重发）
3. 去重机制（消息ID跟踪）
4. 顺序重排（序列号）
5. 分片传输（大消息）
6. 流量控制
7. 拥塞控制

**代码量估计**：500-1000行（基本等于重新实现TCP）

---

### 方案B：TCP连接（当前规划）

**实现示例**：
```cpp
// TCP可靠消息发送
void sendTcpTextMessage(QString toUser, QString content) {
    // 1. 获取或创建连接
    QTcpSocket* socket = m_connectionMgr->getConnection(toUser);
    
    // 2. 序列化消息（Protobuf已集成）
    protocol::TcpMessage tcpMsg;
    tcpMsg.set_type(protocol::TcpMessage::TEXT);
    
    protocol::TextMessage textMsg;
    textMsg.set_message_id(QUuid::createUuid().toString().toStdString());
    textMsg.set_from_user_id(m_userId.toStdString());
    textMsg.set_to_user_id(toUser.toStdString());
    textMsg.set_content(content.toStdString());
    textMsg.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    
    std::string payload;
    textMsg.SerializeToString(&payload);
    tcpMsg.set_payload(payload);
    
    // 3. 发送（Qt自动处理可靠性）
    QByteArray data;
    data.resize(tcpMsg.ByteSizeLong());
    tcpMsg.SerializeToArray(data.data(), data.size());
    socket->write(data);
}
```

**优点**：
- ✅ **消息可靠传输**：Qt自动处理重传
- ✅ **顺序保证**：先发先到
- ✅ **流控和拥塞控制**：操作系统级优化
- ✅ **文件传输简单**：直接流式传输
- ✅ **双向通信**：支持请求-响应模式
- ✅ **成熟稳定**：Qt网络模块久经考验

**缺点**：
- ❌ 连接管理复杂（需要维护连接池）
- ❌ 三次握手延迟（首次连接）
- ❌ 断线重连逻辑

**代码量估计**：300-500行（有Qt和Protobuf支持）

---

### 方案C：混合架构（推荐）

**设计原则**：各取所长

```cpp
// UDP：快速、轻量级、不重要的消息
class UdpChannel {
    void broadcastTypingStatus();      // 打字状态
    void broadcastPresenceChange();    // 在线状态变更
    void sendQuickPing();              // 延迟探测
};

// TCP：可靠、重要、大数据量的消息
class TcpChannel {
    void sendTextMessage();            // 文本消息 ⭐
    void sendFile();                   // 文件传输 ⭐
    void sendFileMetadata();           // 文件元信息
    void requestNsfwCheck();           // NSFW检测请求
};
```

**路由规则**：
| 消息类型 | 协议选择 | 理由 |
|---------|---------|------|
| 文本消息 | TCP | 不能丢失 |
| 文件传输 | TCP | 大数据量 |
| NSFW结果 | TCP | 重要决策 |
| 打字状态 | UDP | 实时性>可靠性 |
| 在线状态 | UDP | 丢了无所谓（有心跳兜底） |
| Ping测试 | UDP | 低延迟探测 |

---

## 决策建议

### 🎯 强烈推荐：方案B（TCP为主）

**理由**：

1. **项目需求驱动**
   - ✅ 需要可靠的1v1文本聊天
   - ✅ 需要传输图片文件（NSFW检测）
   - ✅ 用户期望消息"必达"

2. **开发效率优先**
   - TCP + Qt + Protobuf = 成熟工具链
   - UDP需要大量自研代码（容易出Bug）

3. **质量优先于性能**
   - 聊天应用的核心是"可靠性"不是"速度"
   - TCP的延迟（10-50ms）完全可接受

4. **扩展性考虑**
   - 未来功能（语音、视频）也需要TCP
   - IPMsg风格已过时（90年代设计）

### 🔧 实现路线图

**Sprint 3（2周）**：
```
Week 1: TCP连接管理器
├── TcpConnectionManager类
├── 连接池管理
└── 断线重连

Week 2: 文本消息功能
├── 消息发送/接收
├── UI集成（聊天窗口）
└── 消息历史存储
```

**Sprint 4（可选优化）**：
```
如果性能有瓶颈，再考虑UDP优化
├── 打字状态广播（UDP）
├── 在线状态通知（UDP）
└── 延迟探测（UDP Ping）
```

---

## 参考案例

### 成功案例：微信/Telegram

- **微信PC版**：TCP长连接 + 心跳保活
- **Telegram**：TCP + MTProto协议（自研但基于TCP）
- **WhatsApp**：基于XMPP（TCP）

### 失败案例：纯UDP聊天工具

- **飞秋（FeiQ）**：基于IPMsg协议，消息丢失问题严重
- **早期QQ**：UDP为主，后改为TCP（可靠性问题）

---

## 结论

**建议采用方案B（TCP为主）**，理由：

1. ✅ 满足项目核心需求（可靠聊天 + 文件传输）
2. ✅ 开发效率高（利用现有Qt和Protobuf）
3. ✅ 代码质量高（少造轮子，少Bug）
4. ✅ 用户体验好（消息必达）
5. ✅ 易于扩展（未来功能兼容）

**不建议纯UDP方案**，除非：
- ❌ 你有大量时间自研协议栈
- ❌ 你能接受消息丢失
- ❌ 你不需要文件传输

**可选混合方案（Phase 2）**：
- 基础功能用TCP（消息、文件）
- 辅助功能用UDP（状态通知）

---

**最终答案**：继续按Sprint计划实现TCP方案，UDP已经完美解决节点发现问题，不要过度设计！
