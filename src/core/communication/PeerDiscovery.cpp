/**
 * @file PeerDiscovery.cpp
 * @brief UDP节点发现服务实现
 */

#include "PeerDiscovery.h"
#include "../adapters/ProtobufSerializer.h"
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QDebug>

namespace flykylin {
namespace core {

PeerDiscovery::PeerDiscovery(QObject* parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_broadcastTimer(nullptr)
    , m_timeoutCheckTimer(nullptr)
    , m_udpPort(0)
    , m_tcpPort(0)
    , m_isRunning(false)
    , m_loopbackEnabled(false)  // 默认禁用本地回环
    , m_serializer(std::make_unique<flykylin::adapters::ProtobufSerializer>())
{
    qDebug() << "[PeerDiscovery] Created with Protobuf serializer";
}

PeerDiscovery::~PeerDiscovery()
{
    stop();
    qDebug() << "[PeerDiscovery] Destroyed";
}

bool PeerDiscovery::start(quint16 udpPort, quint16 tcpPort)
{
    if (m_isRunning) {
        qWarning() << "[PeerDiscovery] Already running";
        return false;
    }

    m_udpPort = udpPort;
    m_tcpPort = tcpPort;

    // 创建UDP套接字
    m_socket = new QUdpSocket(this);
    
    // 绑定端口并允许地址重用（多个实例可以监听同一端口）
    if (!m_socket->bind(QHostAddress::AnyIPv4, m_udpPort, 
                        QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint)) {
        qCritical() << "[PeerDiscovery] Failed to bind UDP port" << m_udpPort 
                   << ":" << m_socket->errorString();
        delete m_socket;
        m_socket = nullptr;
        return false;
    }

    qInfo() << "[PeerDiscovery] Listening on UDP port" << m_udpPort;

    // 连接UDP接收信号
    connect(m_socket, &QUdpSocket::readyRead, 
            this, &PeerDiscovery::onDatagramReceived);

    // 创建广播定时器（5秒一次）
    m_broadcastTimer = new QTimer(this);
    m_broadcastTimer->setInterval(kBroadcastInterval);
    connect(m_broadcastTimer, &QTimer::timeout, 
            this, &PeerDiscovery::onBroadcastTimer);
    m_broadcastTimer->start();

    // 创建超时检测定时器（10秒一次）
    m_timeoutCheckTimer = new QTimer(this);
    m_timeoutCheckTimer->setInterval(kTimeoutCheckInterval);
    connect(m_timeoutCheckTimer, &QTimer::timeout, 
            this, &PeerDiscovery::onTimeoutCheckTimer);
    m_timeoutCheckTimer->start();

    m_isRunning = true;

    // 立即发送一次上线广播
    sendBroadcast(1); // MSG_ONLINE = 1
    
    qInfo() << "[PeerDiscovery] Started successfully (TCP port:" << m_tcpPort << ")";
    return true;
}

void PeerDiscovery::stop()
{
    if (!m_isRunning) {
        return;
    }

    // 发送下线广播
    sendBroadcast(2); // MSG_OFFLINE = 2

    // 停止定时器
    if (m_broadcastTimer) {
        m_broadcastTimer->stop();
        m_broadcastTimer->deleteLater();
        m_broadcastTimer = nullptr;
    }

    if (m_timeoutCheckTimer) {
        m_timeoutCheckTimer->stop();
        m_timeoutCheckTimer->deleteLater();
        m_timeoutCheckTimer = nullptr;
    }

    // 关闭套接字
    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }

    // 清空节点列表
    m_peers.clear();
    m_lastSeen.clear();

    m_isRunning = false;
    qInfo() << "[PeerDiscovery] Stopped";
}

void PeerDiscovery::onBroadcastTimer()
{
    sendBroadcast(3); // MSG_HEARTBEAT = 3
}

void PeerDiscovery::onTimeoutCheckTimer()
{
    checkTimeout();
}

void PeerDiscovery::onDatagramReceived()
{
    while (m_socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket->receiveDatagram();
        QHostAddress senderAddress = datagram.senderAddress();
        QByteArray data = datagram.data();

        // 忽略自己发送的消息
        QList<QHostAddress> localAddresses;
        for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
            for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
                localAddresses.append(entry.ip());
            }
        }
        
        // 过滤本地地址（除非启用回环模式）
        if (!m_loopbackEnabled && localAddresses.contains(senderAddress)) {
            continue; // 跳过本地地址
        }

        // 处理接收到的消息
        processReceivedMessage(data, senderAddress);
    }
}

void PeerDiscovery::sendBroadcast(int messageType)
{
    if (!m_socket || !m_isRunning) {
        return;
    }

    // 构造当前节点信息
    PeerNode selfNode;
    selfNode.setUserId("local");  // 发送时不重要，接收方会用IP替换
    selfNode.setUserName(QHostInfo::localHostName());
    selfNode.setHostName(QHostInfo::localHostName());
    selfNode.setIpAddress("0.0.0.0");  // 广播时不重要
    selfNode.setTcpPort(m_tcpPort);
    selfNode.setLastSeen(QDateTime::currentDateTime());
    selfNode.setOnline(true);

    // 使用Protobuf序列化
    std::vector<uint8_t> data;
    
    switch (messageType) {
        case 1: // MSG_ONLINE
            data = m_serializer->serializePeerAnnounce(selfNode);
            break;
        case 2: // MSG_OFFLINE
            data = m_serializer->serializePeerGoodbye(selfNode);
            break;
        case 3: // MSG_HEARTBEAT
            data = m_serializer->serializePeerHeartbeat(selfNode);
            break;
        default:
            qWarning() << "[PeerDiscovery] Unknown message type:" << messageType;
            return;
    }

    if (data.empty()) {
        qWarning() << "[PeerDiscovery] Failed to serialize message";
        return;
    }

    // 转换为QByteArray并发送
    QByteArray message(reinterpret_cast<const char*>(data.data()), data.size());
    
    // 发送到广播地址
    QHostAddress broadcastAddress = QHostAddress::Broadcast;
    qint64 sent = m_socket->writeDatagram(message, broadcastAddress, m_udpPort);
    
    if (sent == -1) {
        qWarning() << "[PeerDiscovery] Failed to send broadcast:" << m_socket->errorString();
    } else {
        qDebug() << "[PeerDiscovery] Sent Protobuf broadcast (type:" << messageType 
                 << ", size:" << sent << "bytes)";
    }
}

void PeerDiscovery::processReceivedMessage(const QByteArray& datagram, 
                                          const QHostAddress& senderAddress)
{
    // 转换为std::vector<uint8_t>
    std::vector<uint8_t> data(datagram.begin(), datagram.end());

    // 验证消息格式
    if (!m_serializer->isValidMessage(data)) {
        qWarning() << "[PeerDiscovery] Invalid Protobuf message from" << senderAddress;
        return;
    }

    // 反序列化节点信息
    std::optional<PeerNode> nodeOpt = m_serializer->deserializePeerMessage(data);
    if (!nodeOpt.has_value()) {
        qWarning() << "[PeerDiscovery] Failed to deserialize message from" << senderAddress;
        return;
    }

    PeerNode node = nodeOpt.value();
    
    // 使用发送者IP作为userId
    QString userId = senderAddress.toString();
    node.setUserId(userId);
    node.setIpAddress(userId);

    qDebug() << "[PeerDiscovery] Received Protobuf message from" << userId 
             << "host:" << node.hostName()
             << "tcp:" << node.tcpPort();

    // 更新最后心跳时间
    QDateTime now = QDateTime::currentDateTime();
    m_lastSeen[userId] = now;
    node.setLastSeen(now);

    // 判断消息类型（通过是否在线状态推断）
    if (!node.isOnline()) {
        // MSG_OFFLINE
        if (m_peers.contains(userId)) {
            qInfo() << "[PeerDiscovery] Peer offline (Protobuf):" << userId;
            m_peers.remove(userId);
            m_lastSeen.remove(userId);
            emit peerOffline(userId);
        }
    } else {
        // MSG_ONLINE or MSG_HEARTBEAT
        bool isNewPeer = !m_peers.contains(userId);
        
        m_peers[userId] = node;

        if (isNewPeer) {
            qInfo() << "[PeerDiscovery] New peer discovered (Protobuf):" << userId << node.hostName();
            emit peerDiscovered(node);
        } else {
            emit peerHeartbeat(userId);
        }
    }
}

void PeerDiscovery::checkTimeout()
{
    QDateTime now = QDateTime::currentDateTime();
    QList<QString> timeoutPeers;

    // 检查所有节点的最后心跳时间
    for (auto it = m_lastSeen.constBegin(); it != m_lastSeen.constEnd(); ++it) {
        const QString& userId = it.key();
        const QDateTime& lastSeen = it.value();

        qint64 elapsed = lastSeen.msecsTo(now);
        if (elapsed > kTimeoutThreshold) {
            timeoutPeers.append(userId);
        }
    }

    // 移除超时节点
    for (const QString& userId : timeoutPeers) {
        qInfo() << "[PeerDiscovery] Peer timeout:" << userId 
                << "last seen" << (now.toMSecsSinceEpoch() - m_lastSeen[userId].toMSecsSinceEpoch()) / 1000 
                << "seconds ago";
        
        m_peers.remove(userId);
        m_lastSeen.remove(userId);
        emit peerOffline(userId);
    }
}

} // namespace core
} // namespace flykylin
