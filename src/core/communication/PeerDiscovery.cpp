/**
 * @file PeerDiscovery.cpp
 * @brief UDP节点发现服务实现
 */

#include "PeerDiscovery.h"
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QDebug>

// TODO: 生成Protobuf头文件后添加
// #include "message.pb.h"

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
{
    qDebug() << "[PeerDiscovery] Created";
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
        
        if (localAddresses.contains(senderAddress)) {
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

    // TODO: 使用Protobuf序列化消息
    // 暂时使用简单的文本协议进行测试
    QByteArray message;
    message.append(QString("FLYKYLIN|%1|%2|%3|%4\n")
                    .arg(messageType)
                    .arg(m_tcpPort)
                    .arg(QHostInfo::localHostName())
                    .arg(QDateTime::currentMSecsSinceEpoch())
                    .toUtf8());

    // 广播到所有网络接口
    QHostAddress broadcastAddress = QHostAddress::Broadcast;
    qint64 sent = m_socket->writeDatagram(message, broadcastAddress, m_udpPort);
    
    if (sent == -1) {
        qWarning() << "[PeerDiscovery] Failed to send broadcast:" << m_socket->errorString();
    } else {
        qDebug() << "[PeerDiscovery] Sent broadcast (type:" << messageType 
                 << ", size:" << sent << "bytes)";
    }
}

void PeerDiscovery::processReceivedMessage(const QByteArray& datagram, 
                                          const QHostAddress& senderAddress)
{
    // TODO: 使用Protobuf反序列化消息
    // 暂时使用简单的文本协议解析
    QString message = QString::fromUtf8(datagram).trimmed();
    QStringList parts = message.split('|');

    if (parts.size() < 5 || parts[0] != "FLYKYLIN") {
        qWarning() << "[PeerDiscovery] Invalid message format:" << message;
        return;
    }

    int messageType = parts[1].toInt();
    quint16 tcpPort = parts[2].toUShort();
    QString hostName = parts[3];
    qint64 timestamp = parts[4].toLongLong();
    Q_UNUSED(timestamp);  // TODO: 将来可用于消息验证和时序检查

    QString userId = senderAddress.toString();

    qDebug() << "[PeerDiscovery] Received from" << userId 
             << "type:" << messageType 
             << "host:" << hostName
             << "tcp:" << tcpPort;

    // 更新最后心跳时间
    QDateTime now = QDateTime::currentDateTime();
    m_lastSeen[userId] = now;

    // 处理不同消息类型
    switch (messageType) {
        case 1: // MSG_ONLINE
        case 3: // MSG_HEARTBEAT
        {
            // 检查是否为新节点
            bool isNewPeer = !m_peers.contains(userId);

            // 创建或更新PeerNode
            PeerNode node;
            node.setUserId(userId);
            node.setUserName(hostName); // 暂时用主机名作为用户名
            node.setHostName(hostName);
            node.setIpAddress(userId);
            node.setTcpPort(tcpPort);
            node.setLastSeen(now);
            node.setOnline(true);

            m_peers[userId] = node;

            if (isNewPeer) {
                qInfo() << "[PeerDiscovery] New peer discovered:" << userId << hostName;
                emit peerDiscovered(node);
            } else {
                emit peerHeartbeat(userId);
            }
            break;
        }
        
        case 2: // MSG_OFFLINE
        {
            if (m_peers.contains(userId)) {
                qInfo() << "[PeerDiscovery] Peer offline:" << userId;
                m_peers.remove(userId);
                m_lastSeen.remove(userId);
                emit peerOffline(userId);
            }
            break;
        }

        default:
            qWarning() << "[PeerDiscovery] Unknown message type:" << messageType;
            break;
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
