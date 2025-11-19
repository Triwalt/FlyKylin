/**
 * @file PeerDiscovery.h
 * @brief UDP节点发现服务
 * 
 * 负责通过UDP广播发现局域网内的其他FlyKylin节点
 * 实现心跳机制和离线检测
 */

#ifndef PEERDISCOVERY_H
#define PEERDISCOVERY_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QMap>
#include <QDateTime>
#include <memory>
#include "../models/PeerNode.h"

// 前向声明
namespace flykylin {
namespace adapters {
    class ProtobufSerializer;
}
}

namespace flykylin {
namespace core {

/**
 * @class PeerDiscovery
 * @brief P2P节点发现服务
 * 
 * 功能：
 * - 每5秒发送UDP广播（MSG_HEARTBEAT）
 * - 接收其他节点的广播并维护在线列表
 * - 检测30秒无心跳的节点并标记为离线
 */
class PeerDiscovery : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit PeerDiscovery(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~PeerDiscovery() override;

    /**
     * @brief 启动节点发现服务
     * @param udpPort UDP监听端口（默认45678）
     * @param tcpPort 本地TCP监听端口（用于通知其他节点）
     * @return 成功返回true，失败返回false
     */
    bool start(quint16 udpPort = 45678, quint16 tcpPort = 45679);

    /**
     * @brief 停止节点发现服务
     */
    void stop();

    /**
     * @brief 检查服务是否正在运行
     * @return 正在运行返回true，否则返回false
     */
    bool isRunning() const { return m_isRunning; }

    /**
     * @brief 获取在线节点数量
     * @return 在线节点数量
     */
    int onlineNodeCount() const { return m_peers.count(); }

    /**
     * @brief 启用/禁用本地回环模式（用于开发测试）
     * @param enable true启用本地回环，false禁用
     */
    void setLoopbackEnabled(bool enable) { m_loopbackEnabled = enable; }

signals:
    /**
     * @brief 发现新节点信号
     * @param node 新发现的节点信息
     */
    void peerDiscovered(const PeerNode& node);

    /**
     * @brief 节点离线信号
     * @param userId 离线节点的用户ID
     */
    void peerOffline(const QString& userId);

    /**
     * @brief 节点心跳更新信号
     * @param userId 节点用户ID
     */
    void peerHeartbeat(const QString& userId);

private slots:
    /**
     * @brief 广播心跳定时器回调
     */
    void onBroadcastTimer();

    /**
     * @brief 超时检测定时器回调
     */
    void onTimeoutCheckTimer();

    /**
     * @brief UDP数据报接收回调
     */
    void onDatagramReceived();

private:
    /**
     * @brief 发送UDP广播消息
     * @param messageType 消息类型（MSG_ONLINE/MSG_HEARTBEAT/MSG_OFFLINE）
     */
    void sendBroadcast(int messageType);

    /**
     * @brief 处理接收到的UDP消息
     * @param datagram 数据报内容
     * @param senderAddress 发送者地址
     */
    void processReceivedMessage(const QByteArray& datagram, const QHostAddress& senderAddress);

    /**
     * @brief 检查并移除超时节点
     */
    void checkTimeout();

private:
    QUdpSocket* m_socket;                      ///< UDP套接字
    QTimer* m_broadcastTimer;                   ///< 广播定时器（5秒）
    QTimer* m_timeoutCheckTimer;                ///< 超时检测定时器（10秒）
    
    quint16 m_udpPort;                          ///< UDP监听端口
    quint16 m_tcpPort;                          ///< 本地TCP端口
    bool m_isRunning;                           ///< 运行状态
    bool m_loopbackEnabled;                     ///< 本地回环模式（开发测试用）
    
    QMap<QString, PeerNode> m_peers;            ///< userId -> PeerNode
    QMap<QString, QDateTime> m_lastSeen;        ///< userId -> 最后心跳时间
    
    std::unique_ptr<flykylin::adapters::ProtobufSerializer> m_serializer;  ///< Protobuf序列化器
    
    static constexpr int kBroadcastInterval = 5000;     ///< 广播间隔（毫秒）
    static constexpr int kTimeoutCheckInterval = 10000;  ///< 超时检测间隔（毫秒）
    static constexpr int kTimeoutThreshold = 30000;     ///< 超时阈值（毫秒）
};

} // namespace core
} // namespace flykylin

#endif // PEERDISCOVERY_H
