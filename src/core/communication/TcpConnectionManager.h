/**
 * @file TcpConnectionManager.h
 * @brief TCP connection pool manager (singleton)
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#pragma once

#include "TcpConnection.h"
#include "MessageQueue.h"
#include <QObject>
#include <QMap>
#include <QTimer>

namespace flykylin {
namespace communication {

/**
 * @brief TCP connection pool manager (singleton pattern)
 * 
 * Features:
 * - Manage up to 20 concurrent TCP connections
 * - Auto-cleanup idle connections (5 minutes timeout)
 * - Per-connection message queue
 * - Thread-safe via Qt signal/slot mechanism
 */
class TcpConnectionManager : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Get singleton instance
     */
    static TcpConnectionManager* instance();
    
    /**
     * @brief Connect to a peer
     * @param peerId Peer user ID
     * @param ip Peer IP address
     * @param port Peer TCP port
     */
    void connectToPeer(const QString& peerId, const QString& ip, quint16 port);
    
    /**
     * @brief Disconnect from a peer
     * @param peerId Peer user ID
     */
    void disconnectFromPeer(const QString& peerId);
    
    /**
     * @brief Send message to peer
     * @param peerId Peer user ID
     * @param data Serialized Protobuf message
     * @param priority Message priority (default: High)
     */
    void sendMessage(const QString& peerId, 
                     const QByteArray& data, 
                     MessageQueue::Priority priority = MessageQueue::Priority::High);
    
    /**
     * @brief Get connection state
     * @param peerId Peer user ID
     * @return Connection state
     */
    ConnectionState getConnectionState(const QString& peerId) const;
    
    /**
     * @brief Get number of active connections
     */
    int activeConnectionCount() const;
    
signals:
    /**
     * @brief Connection state changed
     */
    void connectionStateChanged(QString peerId, ConnectionState state, QString reason);
    
    /**
     * @brief Message received from peer
     */
    void messageReceived(QString peerId, QByteArray data);
    
    /**
     * @brief Message sent successfully
     */
    void messageSent(QString peerId, quint64 messageId);
    
    /**
     * @brief Message send failed
     */
    void messageFailed(QString peerId, quint64 messageId, QString error);
    
private:
    explicit TcpConnectionManager(QObject* parent = nullptr);
    ~TcpConnectionManager() override;
    
    Q_DISABLE_COPY(TcpConnectionManager)
    
private slots:
    void onConnectionStateChanged(ConnectionState state, QString reason);
    void onMessageReceived(const QByteArray& data);
    void onMessageSent(quint64 messageId);
    void onMessageFailed(quint64 messageId, QString error);
    void cleanupIdleConnections();
    void processMessageQueue(const QString& peerId);
    
private:
    TcpConnection* getOrCreateConnection(const QString& peerId, 
                                        const QString& ip, 
                                        quint16 port);
    
    QMap<QString, TcpConnection*> m_connections;  ///< peerId -> Connection
    QMap<QString, MessageQueue*> m_messageQueues; ///< peerId -> Queue
    
    QTimer* m_cleanupTimer;  ///< Cleanup timer (every minute)
    
    static constexpr int kMaxConnections = 20;        ///< Max 20 connections
    static constexpr int kIdleTimeout = 300000;       ///< 5 minutes idle timeout (milliseconds)
    static constexpr int kCleanupInterval = 60000;    ///< 1 minute cleanup interval
};

} // namespace communication
} // namespace flykylin
