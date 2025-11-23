/**
 * @file TcpConnection.h
 * @brief TCP connection with state machine, retry, and heartbeat
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>
#include <QByteArray>

namespace flykylin {
namespace communication {

/**
 * @brief TCP connection state enum
 */
enum class ConnectionState {
    Disconnected,    ///< Not connected
    Connecting,      ///< Connecting in progress
    Connected,       ///< Successfully connected
    Reconnecting,    ///< Reconnecting after failure
    Failed           ///< Connection failed (gave up)
};

/**
 * @brief Handshake state enum
 */
enum class HandshakeState {
    NotStarted,      ///< Handshake not started
    RequestSent,     ///< Handshake request sent, waiting for response
    ResponseSent,    ///< Handshake response sent (server side)
    Completed,       ///< Handshake completed successfully
    Failed           ///< Handshake failed
};

/**
 * @brief Single TCP connection with state machine, retry, and heartbeat
 * 
 * Features:
 * - State machine: 5 states (Disconnected/Connecting/Connected/Reconnecting/Failed)
 * - Exponential backoff retry (5 attempts: 1s/2s/4s/8s/16s)
 * - Heartbeat keepalive (30s interval, 60s timeout)
 * - Message framing: 4-byte length + Protobuf payload
 * - Thread-safe: All operations via Qt signal/slot
 */
class TcpConnection : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Constructor
     * @param peerId Peer user ID
     * @param peerIp Peer IP address
     * @param peerPort Peer TCP port
     * @param parent Parent QObject
     */
    explicit TcpConnection(const QString& peerId, 
                          const QString& peerIp, 
                          quint16 peerPort, 
                          QObject* parent = nullptr);
    
    /**
     * @brief Constructor for an already-connected inbound socket
     * @param peerId Peer user ID (typically derived from peer IP)
     * @param existingSocket Existing QTcpSocket that is already connected
     * @param parent Parent QObject
     */
    explicit TcpConnection(const QString& peerId,
                          QTcpSocket* existingSocket,
                          QObject* parent = nullptr);
    
    /**
     * @brief Destructor - automatically disconnects
     */
    ~TcpConnection() override;
    
    // Connection control
    /**
     * @brief Initiate connection to peer
     */
    void connectToHost();
    
    /**
     * @brief Disconnect from peer
     */
    void disconnectFromHost();
    
    // Message sending
    /**
     * @brief Send message data
     * @param data Serialized Protobuf message
     */
    void sendMessage(const QByteArray& data);
    
    // State query
    /**
     * @brief Get current connection state
     */
    ConnectionState state() const { return m_state; }
    
    /**
     * @brief Get peer ID
     */
    QString peerId() const { return m_peerId; }
    
    /**
     * @brief Get last activity time
     */
    QDateTime lastActivity() const { return m_lastActivity; }
    
signals:
    /**
     * @brief Connection state changed
     * @param newState New connection state
     * @param reason State change reason (for UI display)
     */
    void stateChanged(ConnectionState newState, QString reason);
    
    /**
     * @brief Message received
     * @param data Received message data
     */
    void messageReceived(const QByteArray& data);
    
    /**
     * @brief Message sent successfully
     * @param messageId Message sequence ID
     */
    void messageSent(quint64 messageId);
    
    /**
     * @brief Message send failed
     * @param messageId Message sequence ID
     * @param error Error description
     */
    void messageFailed(quint64 messageId, QString error);
    
    /**
     * @brief Connection error occurred
     * @param error Error description
     */
    void errorOccurred(QString error);
    
    /**
     * @brief Handshake completed successfully
     */
    void handshakeCompleted();
    
    /**
     * @brief Handshake failed
     * @param error Error description
     */
    void handshakeFailed(QString error);
    
private slots:
    // Socket event handlers
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);
    
    // Timer handlers
    void onHeartbeatTimeout();
    void onReconnectTimeout();
    void onHandshakeTimeout();
    
private:
    // State machine
    void setState(ConnectionState newState, const QString& reason);
    
    // Heartbeat mechanism
    void startHeartbeat();
    void stopHeartbeat();
    void sendHeartbeat();
    
    // Reconnect mechanism
    void scheduleReconnect();
    void attemptReconnect();
    
    // Handshake mechanism
    void startHandshake();
    void sendHandshakeRequest();
    void sendHandshakeResponse(bool accepted, const QString& errorMsg = QString());
    void handleHandshakeRequest(const QByteArray& payload);
    void handleHandshakeResponse(const QByteArray& payload);
    
    // Data processing
    void processIncomingData();
    void processTcpMessage(const QByteArray& messageData);
    quint32 readMessageLength();
    
    // Member variables
    QString m_peerId;              ///< Peer user ID
    QString m_peerIp;              ///< Peer IP address
    quint16 m_peerPort;            ///< Peer TCP port
    
    ConnectionState m_state;       ///< Current connection state
    QTcpSocket* m_socket;          ///< TCP socket
    
    QTimer* m_heartbeatTimer;      ///< Heartbeat timer (30s)
    QTimer* m_reconnectTimer;      ///< Reconnect timer
    QTimer* m_handshakeTimer;      ///< Handshake timeout timer
    
    int m_retryCount;              ///< Retry attempt counter
    QDateTime m_lastActivity;      ///< Last activity timestamp
    
    HandshakeState m_handshakeState;  ///< Handshake state
    QString m_peerName;            ///< Peer username (after handshake)
    
    QByteArray m_receiveBuffer;    ///< Receive buffer
    quint64 m_nextSequence;        ///< Next message sequence number
    
    bool m_isIncoming;             ///< True if this connection was accepted by TcpServer
    
    // Constants
    static constexpr int kHeartbeatInterval = 30000;  ///< 30 seconds
    static constexpr int kTimeoutThreshold = 60000;   ///< 60 seconds timeout
    static constexpr int kHandshakeTimeout = 5000;    ///< 5 seconds handshake timeout
};

} // namespace communication
} // namespace flykylin
