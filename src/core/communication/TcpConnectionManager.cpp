/**
 * @file TcpConnectionManager.cpp
 * @brief TCP connection pool manager implementation
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#include "TcpConnectionManager.h"
#include "../models/PeerNode.h"
#include "../communication/PeerDiscovery.h"
#include <QDebug>
#include <QMetaObject>

namespace flykylin {
namespace communication {

TcpConnectionManager* TcpConnectionManager::instance() {
    static TcpConnectionManager* s_instance = new TcpConnectionManager();
    return s_instance;
}

void TcpConnectionManager::addIncomingConnection(const QString& peerId, QTcpSocket* socket) {
    if (!socket) {
        qWarning() << "[TcpConnectionManager] addIncomingConnection called with null socket for" << peerId;
        return;
    }

    // Enforce connection limit for new peers
    if (m_connections.size() >= kMaxConnections && !m_connections.contains(peerId)) {
        qCritical() << "[TcpConnectionManager] Connection limit reached (" << kMaxConnections
                    << "), rejecting incoming connection from" << peerId;
        socket->close();
        socket->deleteLater();
        emit connectionStateChanged(peerId, ConnectionState::Failed,
                                   QStringLiteral("Connection limit reached"));
        return;
    }

    // If we already have a connection for this peer, prefer the existing one and close the new socket
    if (m_connections.contains(peerId)) {
        qInfo() << "[TcpConnectionManager] Incoming connection for existing peer" << peerId
                << "- closing new socket";
        socket->close();
        socket->deleteLater();
        return;
    }

    qInfo() << "[TcpConnectionManager] Registering incoming connection for" << peerId;

    TcpConnection* conn = new TcpConnection(peerId, socket, this);

    // Connect signals
    connect(conn, &TcpConnection::stateChanged,
            this, &TcpConnectionManager::onConnectionStateChanged);
    connect(conn, &TcpConnection::messageReceived,
            this, &TcpConnectionManager::onMessageReceived);
    connect(conn, &TcpConnection::messageSent,
            this, &TcpConnectionManager::onMessageSent);
    connect(conn, &TcpConnection::messageFailed,
            this, &TcpConnectionManager::onMessageFailed);

    m_connections[peerId] = conn;

    // For an accepted socket we are already connected; emit initial state
    emit connectionStateChanged(peerId, ConnectionState::Connected,
                               QStringLiteral("Incoming connection accepted"));
}

TcpConnectionManager::TcpConnectionManager(QObject* parent)
    : QObject(parent)
    , m_cleanupTimer(new QTimer(this))
{
    // Setup cleanup timer
    m_cleanupTimer->setInterval(kCleanupInterval);
    connect(m_cleanupTimer, &QTimer::timeout, this, &TcpConnectionManager::cleanupIdleConnections);
    m_cleanupTimer->start();
    
    qInfo() << "[TcpConnectionManager] Initialized";
}

TcpConnectionManager::~TcpConnectionManager() {
    // Disconnect all connections
    for (auto* conn : m_connections) {
        conn->disconnectFromHost();
        conn->deleteLater();
    }
    m_connections.clear();
    
    // Delete all queues
    for (auto* queue : m_messageQueues) {
        queue->deleteLater();
    }
    m_messageQueues.clear();
    
    qInfo() << "[TcpConnectionManager] Destroyed";
}

void TcpConnectionManager::setupPeerDiscovery(QObject* peerDiscovery) {
    if (!peerDiscovery) {
        qWarning() << "[TcpConnectionManager] setupPeerDiscovery: null peerDiscovery";
        return;
    }

    auto* discovery = qobject_cast<flykylin::core::PeerDiscovery*>(peerDiscovery);
    if (!discovery) {
        qWarning() << "[TcpConnectionManager] setupPeerDiscovery: object is not a PeerDiscovery instance";
        return;
    }

    bool connectedDiscovered = connect(discovery,
                                       &flykylin::core::PeerDiscovery::peerDiscovered,
                                       this,
                                       &TcpConnectionManager::onPeerDiscovered);

    bool connectedOffline = connect(discovery,
                                    &flykylin::core::PeerDiscovery::peerOffline,
                                    this,
                                    &TcpConnectionManager::onPeerOffline);

    if (connectedDiscovered && connectedOffline) {
        qInfo() << "[TcpConnectionManager] PeerDiscovery integration enabled";
    } else {
        qWarning() << "[TcpConnectionManager] Failed to connect to PeerDiscovery";
    }
}

void TcpConnectionManager::onPeerDiscovered(const flykylin::core::PeerNode& node) {
    QString peerId = node.userId();
    QString ip = node.ipAddress().toString();
    quint16 port = node.tcpPort();
    
    qInfo() << "[TcpConnectionManager] Peer discovered:" << peerId 
            << "at" << ip << ":" << port << "- Auto-connecting...";
    
    // Auto-connect to the discovered peer
    connectToPeer(peerId, ip, port);
}

void TcpConnectionManager::onPeerOffline(const QString& userId)
{
    qInfo() << "[TcpConnectionManager] Peer offline:" << userId << "- disconnecting TCP connection";
    disconnectFromPeer(userId);
}

void TcpConnectionManager::connectToPeer(const QString& peerId, const QString& ip, quint16 port) {
    qInfo() << "[TcpConnectionManager] Connect to peer" << peerId << ip << port;
    
    // Check connection limit
    if (m_connections.size() >= kMaxConnections && !m_connections.contains(peerId)) {
        qCritical() << "[TcpConnectionManager] Connection limit reached (" << kMaxConnections 
                    << "), cannot connect to" << peerId;
        emit connectionStateChanged(peerId, ConnectionState::Failed, 
                                   "Connection limit reached");
        return;
    }
    
    TcpConnection* conn = getOrCreateConnection(peerId, ip, port);
    conn->connectToHost();
}

void TcpConnectionManager::disconnectFromPeer(const QString& peerId) {
    qInfo() << "[TcpConnectionManager] Disconnect from peer" << peerId;
    
    if (!m_connections.contains(peerId)) {
        qWarning() << "[TcpConnectionManager] No connection for peer" << peerId;
        return;
    }
    
    TcpConnection* conn = m_connections[peerId];
    conn->disconnectFromHost();
    
    // Remove connection
    m_connections.remove(peerId);
    conn->deleteLater();
    
    // Clear message queue
    if (m_messageQueues.contains(peerId)) {
        MessageQueue* queue = m_messageQueues[peerId];
        m_messageQueues.remove(peerId);
        queue->deleteLater();
    }
}

void TcpConnectionManager::sendMessage(const QString& peerId, 
                                      const QByteArray& data, 
                                      MessageQueue::Priority priority) {
    if (!m_connections.contains(peerId)) {
        qWarning() << "[TcpConnectionManager] No connection for peer" << peerId;
        emit messageFailed(peerId, 0, "Not connected");
        return;
    }
    
    // Get or create message queue
    MessageQueue* queue = m_messageQueues.value(peerId, nullptr);
    if (!queue) {
        queue = new MessageQueue(this);
        m_messageQueues[peerId] = queue;
        connect(queue, &MessageQueue::messageEnqueued, 
                this, [this, peerId]() { processMessageQueue(peerId); });
    }
    
    TcpConnection* conn = m_connections[peerId];
    
    // If connected, send directly
    if (conn->state() == ConnectionState::Connected) {
        conn->sendMessage(data);
    } else {
        // Otherwise, queue the message
        queue->enqueue(data, priority);
        qDebug() << "[TcpConnectionManager] Message queued for" << peerId 
                 << "queue_size=" << queue->size();
    }
}

ConnectionState TcpConnectionManager::getConnectionState(const QString& peerId) const {
    if (!m_connections.contains(peerId)) {
        return ConnectionState::Disconnected;
    }
    return m_connections[peerId]->state();
}

int TcpConnectionManager::activeConnectionCount() const {
    int count = 0;
    for (const auto* conn : m_connections) {
        if (conn->state() == ConnectionState::Connected) {
            count++;
        }
    }
    return count;
}

void TcpConnectionManager::onConnectionStateChanged(ConnectionState state, QString reason) {
    TcpConnection* conn = qobject_cast<TcpConnection*>(sender());
    if (!conn) {
        return;
    }
    
    QString peerId = conn->peerId();
    qInfo() << "[TcpConnectionManager] Connection state changed for" << peerId 
            << "new_state=" << static_cast<int>(state) << "reason=" << reason;
    
    emit connectionStateChanged(peerId, state, reason);
    
    // If connected, process queued messages
    if (state == ConnectionState::Connected) {
        processMessageQueue(peerId);
    }
}

void TcpConnectionManager::onMessageReceived(const QByteArray& data) {
    TcpConnection* conn = qobject_cast<TcpConnection*>(sender());
    if (!conn) {
        return;
    }
    
    QString peerId = conn->peerId();
    qDebug() << "[TcpConnectionManager] Message received from" << peerId 
             << "size=" << data.size();
    
    emit messageReceived(peerId, data);
}

void TcpConnectionManager::onMessageSent(quint64 messageId) {
    TcpConnection* conn = qobject_cast<TcpConnection*>(sender());
    if (!conn) {
        return;
    }
    
    QString peerId = conn->peerId();
    emit messageSent(peerId, messageId);
}

void TcpConnectionManager::onMessageFailed(quint64 messageId, QString error) {
    TcpConnection* conn = qobject_cast<TcpConnection*>(sender());
    if (!conn) {
        return;
    }
    
    QString peerId = conn->peerId();
    qWarning() << "[TcpConnectionManager] Message failed for" << peerId 
               << "id=" << messageId << "error=" << error;
    
    emit messageFailed(peerId, messageId, error);
}

void TcpConnectionManager::cleanupIdleConnections() {
    QDateTime now = QDateTime::currentDateTime();
    QList<QString> toRemove;
    
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        TcpConnection* conn = it.value();
        
        // Check if idle timeout
        qint64 idleTime = conn->lastActivity().msecsTo(now);
        if (idleTime > kIdleTimeout && conn->state() == ConnectionState::Connected) {
            qInfo() << "[TcpConnectionManager] Connection idle timeout for" << it.key() 
                    << "idle_time=" << idleTime << "ms";
            toRemove.append(it.key());
        }
    }
    
    // Remove idle connections
    for (const QString& peerId : toRemove) {
        disconnectFromPeer(peerId);
    }
    
    if (!toRemove.isEmpty()) {
        qInfo() << "[TcpConnectionManager] Cleaned up" << toRemove.size() 
                << "idle connections, active=" << activeConnectionCount();
    }
}

void TcpConnectionManager::processMessageQueue(const QString& peerId) {
    if (!m_messageQueues.contains(peerId)) {
        return;
    }
    
    MessageQueue* queue = m_messageQueues[peerId];
    TcpConnection* conn = m_connections.value(peerId, nullptr);
    
    if (!conn || conn->state() != ConnectionState::Connected) {
        qDebug() << "[TcpConnectionManager] Cannot process queue, not connected:" << peerId;
        return;
    }
    
    // Send all queued messages
    while (!queue->isEmpty()) {
        MessageQueue::QueuedMessage msg = queue->dequeue();
        
        qInfo() << "[TcpConnectionManager] Sending queued message for" << peerId 
                << "id=" << msg.messageId;
        
        conn->sendMessage(msg.data);
    }
}

TcpConnection* TcpConnectionManager::getOrCreateConnection(const QString& peerId, 
                                                          const QString& ip, 
                                                          quint16 port) {
    if (m_connections.contains(peerId)) {
        return m_connections[peerId];
    }
    
    qInfo() << "[TcpConnectionManager] Creating new connection for" << peerId;
    
    TcpConnection* conn = new TcpConnection(peerId, ip, port, this);
    
    // Connect signals
    connect(conn, &TcpConnection::stateChanged, 
            this, &TcpConnectionManager::onConnectionStateChanged);
    connect(conn, &TcpConnection::messageReceived, 
            this, &TcpConnectionManager::onMessageReceived);
    connect(conn, &TcpConnection::messageSent, 
            this, &TcpConnectionManager::onMessageSent);
    connect(conn, &TcpConnection::messageFailed, 
            this, &TcpConnectionManager::onMessageFailed);
    
    m_connections[peerId] = conn;
    
    return conn;
}

} // namespace communication
} // namespace flykylin
