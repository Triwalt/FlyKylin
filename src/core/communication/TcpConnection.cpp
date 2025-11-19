/**
 * @file TcpConnection.cpp
 * @brief TCP connection implementation with state machine
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#include "TcpConnection.h"
#include "RetryStrategy.h"
#include <QDataStream>
#include <QHostAddress>
#include <QDebug>

namespace flykylin {
namespace communication {

TcpConnection::TcpConnection(const QString& peerId, 
                             const QString& peerIp, 
                             quint16 peerPort, 
                             QObject* parent)
    : QObject(parent)
    , m_peerId(peerId)
    , m_peerIp(peerIp)
    , m_peerPort(peerPort)
    , m_state(ConnectionState::Disconnected)
    , m_socket(new QTcpSocket(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
    , m_retryCount(0)
    , m_nextSequence(0)
{
    // Configure socket
    connect(m_socket, &QTcpSocket::connected, this, &TcpConnection::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpConnection::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpConnection::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpConnection::onSocketError);
    
    // Configure heartbeat timer
    m_heartbeatTimer->setInterval(kHeartbeatInterval);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &TcpConnection::onHeartbeatTimeout);
    
    // Configure reconnect timer (single-shot)
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &TcpConnection::onReconnectTimeout);
    
    qInfo() << "[TcpConnection]" << m_peerId << "created";
}

TcpConnection::~TcpConnection() {
    disconnectFromHost();
    qInfo() << "[TcpConnection]" << m_peerId << "destroyed";
}

void TcpConnection::connectToHost() {
    if (m_state == ConnectionState::Connected || m_state == ConnectionState::Connecting) {
        qWarning() << "[TcpConnection]" << m_peerId << "already connected or connecting";
        return;
    }
    
    qInfo() << "[TcpConnection]" << m_peerId << "connecting to" << m_peerIp << ":" << m_peerPort;
    
    setState(ConnectionState::Connecting, "Connecting...");
    m_retryCount = 0;
    m_socket->connectToHost(QHostAddress(m_peerIp), m_peerPort);
}

void TcpConnection::disconnectFromHost() {
    if (m_state == ConnectionState::Disconnected) {
        return;
    }
    
    qInfo() << "[TcpConnection]" << m_peerId << "disconnecting";
    
    stopHeartbeat();
    m_reconnectTimer->stop();
    
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
    
    setState(ConnectionState::Disconnected, "Disconnected by user");
}

void TcpConnection::sendMessage(const QByteArray& data) {
    if (m_state != ConnectionState::Connected) {
        QString error = QString("Cannot send: not connected (state=%1)").arg(static_cast<int>(m_state));
        qWarning() << "[TcpConnection]" << m_peerId << error;
        emit messageFailed(m_nextSequence, error);
        return;
    }
    
    // Message frame: [4-byte length][protobuf payload]
    QByteArray frame;
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    
    quint32 length = static_cast<quint32>(data.size());
    stream << length;
    frame.append(data);
    
    qint64 written = m_socket->write(frame);
    if (written == -1) {
        QString error = QString("Write failed: %1").arg(m_socket->errorString());
        qCritical() << "[TcpConnection]" << m_peerId << error;
        emit messageFailed(m_nextSequence, error);
        return;
    }
    
    m_socket->flush();
    m_lastActivity = QDateTime::currentDateTime();
    
    quint64 messageId = m_nextSequence++;
    emit messageSent(messageId);
    
    qDebug() << "[TcpConnection]" << m_peerId << "message sent, id=" << messageId << "size=" << data.size();
}

void TcpConnection::setState(ConnectionState newState, const QString& reason) {
    if (m_state == newState) {
        return;
    }
    
    ConnectionState oldState = m_state;
    m_state = newState;
    
    qInfo() << "[TcpConnection]" << m_peerId << "state change:" 
            << static_cast<int>(oldState) << "->" << static_cast<int>(newState)
            << "reason:" << reason;
    
    emit stateChanged(newState, reason);
}

void TcpConnection::onConnected() {
    qInfo() << "[TcpConnection]" << m_peerId << "connected successfully";
    
    setState(ConnectionState::Connected, "Connected");
    m_retryCount = 0;
    m_lastActivity = QDateTime::currentDateTime();
    
    startHeartbeat();
}

void TcpConnection::onDisconnected() {
    qWarning() << "[TcpConnection]" << m_peerId << "disconnected";
    
    stopHeartbeat();
    
    // Check if this is intentional disconnect
    if (m_state == ConnectionState::Disconnected) {
        return;  // User-initiated disconnect
    }
    
    // Unexpected disconnect - attempt reconnect
    if (m_state == ConnectionState::Connected || m_state == ConnectionState::Connecting) {
        setState(ConnectionState::Reconnecting, "Connection lost, reconnecting...");
        scheduleReconnect();
    }
}

void TcpConnection::onReadyRead() {
    m_receiveBuffer.append(m_socket->readAll());
    m_lastActivity = QDateTime::currentDateTime();
    
    processIncomingData();
}

void TcpConnection::onSocketError(QAbstractSocket::SocketError error) {
    QString errorString = m_socket->errorString();
    qCritical() << "[TcpConnection]" << m_peerId << "socket error:" << error << errorString;
    
    emit errorOccurred(errorString);
    
    // Handle connection errors
    if (m_state == ConnectionState::Connecting) {
        setState(ConnectionState::Reconnecting, QString("Connection failed: %1").arg(errorString));
        scheduleReconnect();
    }
}

void TcpConnection::onHeartbeatTimeout() {
    qint64 elapsed = m_lastActivity.msecsTo(QDateTime::currentDateTime());
    
    if (elapsed > kTimeoutThreshold) {
        qWarning() << "[TcpConnection]" << m_peerId << "heartbeat timeout, disconnecting";
        disconnectFromHost();
        setState(ConnectionState::Reconnecting, "Heartbeat timeout");
        scheduleReconnect();
    } else {
        sendHeartbeat();
    }
}

void TcpConnection::onReconnectTimeout() {
    attemptReconnect();
}

void TcpConnection::startHeartbeat() {
    qDebug() << "[TcpConnection]" << m_peerId << "heartbeat started";
    m_heartbeatTimer->start();
}

void TcpConnection::stopHeartbeat() {
    qDebug() << "[TcpConnection]" << m_peerId << "heartbeat stopped";
    m_heartbeatTimer->stop();
}

void TcpConnection::sendHeartbeat() {
    // Send empty message as heartbeat (just the length header with 0)
    QByteArray frame;
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint32>(0);  // Zero-length message = heartbeat
    
    m_socket->write(frame);
    m_socket->flush();
    m_lastActivity = QDateTime::currentDateTime();
    
    qDebug() << "[TcpConnection]" << m_peerId << "heartbeat sent";
}

void TcpConnection::scheduleReconnect() {
    int delay = RetryStrategy::getNextRetryDelay(m_retryCount);
    
    if (delay == -1) {
        qCritical() << "[TcpConnection]" << m_peerId << "reconnect attempts exhausted, giving up";
        setState(ConnectionState::Failed, "Connection failed after max retries");
        return;
    }
    
    QString message = RetryStrategy::getRetryMessage(m_retryCount);
    qInfo() << "[TcpConnection]" << m_peerId << "scheduling reconnect in" << delay << "ms, attempt" << m_retryCount;
    
    setState(ConnectionState::Reconnecting, message);
    m_reconnectTimer->start(delay);
}

void TcpConnection::attemptReconnect() {
    m_retryCount++;
    
    qInfo() << "[TcpConnection]" << m_peerId << "attempting reconnect, retry=" << m_retryCount;
    
    setState(ConnectionState::Connecting, RetryStrategy::getRetryMessage(m_retryCount));
    m_socket->connectToHost(QHostAddress(m_peerIp), m_peerPort);
}

void TcpConnection::processIncomingData() {
    while (m_receiveBuffer.size() >= 4) {
        quint32 messageLength = readMessageLength();
        
        // Check if full message is available
        if (m_receiveBuffer.size() < static_cast<int>(4 + messageLength)) {
            break;  // Wait for more data
        }
        
        // Remove length header
        m_receiveBuffer.remove(0, 4);
        
        // Handle heartbeat (zero-length message)
        if (messageLength == 0) {
            qDebug() << "[TcpConnection]" << m_peerId << "heartbeat received";
            m_lastActivity = QDateTime::currentDateTime();
            continue;
        }
        
        // Extract message payload
        QByteArray messageData = m_receiveBuffer.left(messageLength);
        m_receiveBuffer.remove(0, messageLength);
        
        qDebug() << "[TcpConnection]" << m_peerId << "message received, size=" << messageData.size();
        emit messageReceived(messageData);
    }
}

quint32 TcpConnection::readMessageLength() {
    QDataStream stream(m_receiveBuffer);
    stream.setByteOrder(QDataStream::BigEndian);
    
    quint32 length = 0;
    stream >> length;
    return length;
}

} // namespace communication
} // namespace flykylin
