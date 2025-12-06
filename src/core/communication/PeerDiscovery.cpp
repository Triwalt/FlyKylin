/**
 * @file PeerDiscovery.cpp
 * @brief UDP节点发现服务实现
 */

#include "PeerDiscovery.h"
#include "NetworkInterfaceCache.h"
#include "../adapters/ProtobufSerializer.h"
#include "../config/UserProfile.h"
#include "../database/DatabaseService.h"
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
    , m_networkCache(new flykylin::communication::NetworkInterfaceCache(this, 30000))  // 30s refresh
{
    qDebug() << "[PeerDiscovery] Created with Protobuf serializer and network cache";
}

PeerDiscovery::~PeerDiscovery()
{
    stop();
    // m_networkCache is deleted automatically by Qt parent-child relationship
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
    
    // 启动网络接口缓存（性能优化）
    m_networkCache->start();
    qInfo() << "[PeerDiscovery] Network interface cache started";

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

        // 忽略自己发送的消息（使用缓存优化性能）
        // 过滤本地地址（除非启用回环模式）
        if (!m_loopbackEnabled && m_networkCache->isLocalAddress(senderAddress)) {
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
    const auto& profile = flykylin::core::UserProfile::instance();

    PeerNode selfNode;
    // 使用稳定的UserProfile UUID作为全局唯一userId，避免同一IP多实例冲突
    selfNode.setUserId(profile.userId());
    // 用户名优先使用配置中的昵称，否则退回到主机名
    QString localName = profile.userName();
    if (localName.isEmpty()) {
        localName = QHostInfo::localHostName();
    }
    selfNode.setUserName(localName);
    selfNode.setHostName(QHostInfo::localHostName());
    // 广播包内的IP可以是占位，实际IP以接收端看到的senderAddress为准
    selfNode.setIpAddress("0.0.0.0");
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
    
    // 发送到所有网络接口的广播地址（子网广播更可靠）
    // 为每个接口创建绑定到该接口IP的socket，确保广播从正确的接口发出
    qint64 totalSent = 0;
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : interfaces) {
        if (!(iface.flags() & QNetworkInterface::IsUp) ||
            !(iface.flags() & QNetworkInterface::IsRunning) ||
            (iface.flags() & QNetworkInterface::IsLoopBack)) {
            continue;
        }
        
        for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) {
                continue;
            }
            
            QHostAddress broadcast = entry.broadcast();
            if (broadcast.isNull()) {
                continue;
            }
            
            // 创建绑定到该接口IP的临时socket，确保广播从正确的接口发出
            QUdpSocket tempSocket;
            // 必须先设置socket选项，再绑定
            tempSocket.setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
            if (!tempSocket.bind(entry.ip(), 0)) {
                qWarning() << "[PeerDiscovery] Failed to bind to" << entry.ip().toString()
                           << ":" << tempSocket.errorString();
                continue;
            }
            
            // 发送广播（QUdpSocket默认支持广播，无需额外设置）
            qint64 sent = tempSocket.writeDatagram(message, broadcast, m_udpPort);
            if (sent > 0) {
                totalSent += sent;
                qDebug() << "[PeerDiscovery] Sent broadcast to" << broadcast.toString() 
                         << "from" << entry.ip().toString()
                         << "via" << iface.humanReadableName();
            } else {
                qWarning() << "[PeerDiscovery] Failed to send to" << broadcast.toString()
                           << ":" << tempSocket.errorString();
            }
            tempSocket.close();
        }
    }
    
    // 同时使用主socket发送到全局广播地址作为备用
    qint64 globalSent = m_socket->writeDatagram(message, QHostAddress::Broadcast, m_udpPort);
    if (globalSent > 0) {
        totalSent += globalSent;
        qDebug() << "[PeerDiscovery] Sent global broadcast (255.255.255.255)";
    }
    
    // 额外：发送到已知的对端IP（如果有历史记录）
    // 这是为了解决某些网络环境下广播不可靠的问题
    for (auto it = m_peers.constBegin(); it != m_peers.constEnd(); ++it) {
        const PeerNode& peer = it.value();
        QHostAddress peerAddr(peer.ipAddress().toString());
        if (!peerAddr.isNull() && !m_networkCache->isLocalAddress(peerAddr)) {
            qint64 directSent = m_socket->writeDatagram(message, peerAddr, m_udpPort);
            if (directSent > 0) {
                totalSent += directSent;
                qDebug() << "[PeerDiscovery] Sent direct to known peer" << peerAddr.toString();
            }
        }
    }
    
    if (totalSent == 0) {
        qWarning() << "[PeerDiscovery] Failed to send any broadcast:" << m_socket->errorString();
    } else {
        qDebug() << "[PeerDiscovery] Sent Protobuf broadcast (type:" << messageType 
                 << ", total size:" << totalSent << "bytes)";
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

    // 使用广播中携带的userId作为全局唯一标识（来自对端UserProfile UUID 或实例ID）
    QString userId = node.userId();

    // 忽略自身实例发送的广播（避免单实例把自己当作远端节点）
    const QString localUserId = UserProfile::instance().userId();
    if (userId == localUserId) {
        qDebug() << "[PeerDiscovery] Ignoring self broadcast from" << senderAddress
                 << "userId=" << userId;
        return;
    }

    // 使用实际发送者IP作为节点的当前IP地址
    QString ipString = senderAddress.toString();
    node.setIpAddress(ipString);

    qDebug() << "[PeerDiscovery] Received Protobuf message from" << userId 
             << "host:" << node.hostName()
             << "tcp:" << node.tcpPort();

    // 更新最后心跳时间
    QDateTime now = QDateTime::currentDateTime();
    m_lastSeen[userId] = now;
    node.setLastSeen(now);

    flykylin::database::DatabaseService::PeerInfo info;
    info.userId = userId;
    info.userName = node.userName();
    info.hostName = node.hostName();
    info.ipAddress = node.ipAddress().toString();
    info.tcpPort = node.tcpPort();
    info.lastSeen = now.toMSecsSinceEpoch();
    flykylin::database::DatabaseService::instance()->upsertPeer(info);

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
        bool nameChanged = false;

        if (!isNewPeer) {
            const PeerNode& oldPeer = m_peers[userId];
            nameChanged = (oldPeer.userName() != node.userName()) ||
                          (oldPeer.hostName() != node.hostName());
        }

        m_peers[userId] = node;

        if (isNewPeer || nameChanged) {
            if (isNewPeer) {
                qInfo() << "[PeerDiscovery] New peer discovered (Protobuf):" << userId << node.hostName();
            } else {
                qInfo() << "[PeerDiscovery] Peer updated (Protobuf):" << userId << node.userName();
            }
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
