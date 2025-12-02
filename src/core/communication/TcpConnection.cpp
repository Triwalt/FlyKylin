/**
 * @file TcpConnection.cpp
 * @brief TCP connection implementation with state machine
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#include "TcpConnection.h"
#include "RetryStrategy.h"
#include "../config/UserProfile.h"
#include <QDataStream>
#include <QHostAddress>
#include <QDebug>
#include <QNetworkProxy>
#include <string>
#include "messages.pb.h"

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
    , m_handshakeTimer(new QTimer(this))
    , m_retryCount(0)
    , m_nextSequence(0)
    , m_handshakeState(HandshakeState::NotStarted)
    , m_isIncoming(false)
{
    // Configure socket (disable any system proxy for raw TCP)
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    m_socket->setProxy(proxy);

    // Configure socket
    connect(m_socket, &QTcpSocket::connected, this, &TcpConnection::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpConnection::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpConnection::onReadyRead);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpConnection::onSocketError);
#else
    connect(m_socket,
            static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
            this,
            &TcpConnection::onSocketError);
#endif
    
    // Configure heartbeat timer
    m_heartbeatTimer->setInterval(kHeartbeatInterval);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &TcpConnection::onHeartbeatTimeout);
    
    // Configure reconnect timer (single-shot)
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &TcpConnection::onReconnectTimeout);
    
    // Configure handshake timer (single-shot)
    m_handshakeTimer->setSingleShot(true);
    m_handshakeTimer->setInterval(kHandshakeTimeout);
    connect(m_handshakeTimer, &QTimer::timeout, this, &TcpConnection::onHandshakeTimeout);
    
    qInfo() << "[TcpConnection]" << m_peerId << "created";
}

TcpConnection::TcpConnection(const QString& peerId,
                             QTcpSocket* existingSocket,
                             QObject* parent)
    : QObject(parent)
    , m_peerId(peerId)
    , m_peerIp(existingSocket ? existingSocket->peerAddress().toString() : QString())
    , m_peerPort(existingSocket ? static_cast<quint16>(existingSocket->peerPort()) : 0)
    , m_state(ConnectionState::Connected)
    , m_socket(existingSocket)
    , m_heartbeatTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
    , m_handshakeTimer(new QTimer(this))
    , m_retryCount(0)
    , m_nextSequence(0)
    , m_handshakeState(HandshakeState::NotStarted)
    , m_isIncoming(true)
{
    if (m_socket) {
        m_socket->setParent(this);

        // Disable any system proxy for raw TCP
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::NoProxy);
        m_socket->setProxy(proxy);

        connect(m_socket, &QTcpSocket::connected, this, &TcpConnection::onConnected);
        connect(m_socket, &QTcpSocket::disconnected, this, &TcpConnection::onDisconnected);
        connect(m_socket, &QTcpSocket::readyRead, this, &TcpConnection::onReadyRead);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpConnection::onSocketError);
#else
        connect(m_socket,
                static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
                this,
                &TcpConnection::onSocketError);
#endif

        m_lastActivity = QDateTime::currentDateTime();
    } else {
        m_state = ConnectionState::Failed;
    }

    // Configure heartbeat timer
    m_heartbeatTimer->setInterval(kHeartbeatInterval);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &TcpConnection::onHeartbeatTimeout);

    // Configure reconnect timer (single-shot)
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &TcpConnection::onReconnectTimeout);

    // Configure handshake timer (single-shot)
    m_handshakeTimer->setSingleShot(true);
    m_handshakeTimer->setInterval(kHandshakeTimeout);
    connect(m_handshakeTimer, &QTimer::timeout, this, &TcpConnection::onHandshakeTimeout);

    qInfo() << "[TcpConnection]" << m_peerId << "created (incoming) from"
            << m_peerIp << ":" << m_peerPort;
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
    
    // Check handshake completed
    if (m_handshakeState != HandshakeState::Completed) {
        QString error = QString("Cannot send: handshake not completed (state=%1)").arg(static_cast<int>(m_handshakeState));
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

    // Start protobuf-based handshake
    m_handshakeState = HandshakeState::NotStarted;
    startHandshake();
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
        
        // Process TcpMessage
        processTcpMessage(messageData);
    }
}

quint32 TcpConnection::readMessageLength() {
    QDataStream stream(m_receiveBuffer);
    stream.setByteOrder(QDataStream::BigEndian);
    
    quint32 length = 0;
    stream >> length;
    return length;
}

void TcpConnection::processTcpMessage(const QByteArray& messageData) {
    flykylin::protocol::TcpMessage tcpMessage;
    if (!tcpMessage.ParseFromArray(messageData.constData(), messageData.size())) {
        qWarning() << "[TcpConnection]" << m_peerId
                   << "Failed to parse TcpMessage, size=" << messageData.size();
        return;
    }

    switch (tcpMessage.type()) {
    case flykylin::protocol::TcpMessage::HANDSHAKE_REQUEST: {
        QByteArray payload = QByteArray::fromStdString(tcpMessage.payload());
        handleHandshakeRequest(payload);
        break;
    }
    case flykylin::protocol::TcpMessage::HANDSHAKE_RESPONSE: {
        QByteArray payload = QByteArray::fromStdString(tcpMessage.payload());
        handleHandshakeResponse(payload);
        break;
    }
    default:
        // Application-level message; require handshake to be completed
        if (m_handshakeState != HandshakeState::Completed) {
            qWarning() << "[TcpConnection]" << m_peerId
                       << "Received application message before handshake completed, type="
                       << tcpMessage.type();
            return;
        }

        // Forward the raw TcpMessage bytes to upper layers (MessageService expects this)
        emit messageReceived(messageData);
        break;
    }
}

void TcpConnection::startHandshake() {
    if (m_handshakeState == HandshakeState::Completed) {
        qDebug() << "[TcpConnection]" << m_peerId << "Handshake already completed, skip";
        return;
    }

    qInfo() << "[TcpConnection]" << m_peerId << "Starting protobuf handshake";

    m_handshakeState = HandshakeState::RequestSent;
    sendHandshakeRequest();
    m_handshakeTimer->start();
}

void TcpConnection::sendHandshakeRequest() {
    if (m_state != ConnectionState::Connected) {
        qWarning() << "[TcpConnection]" << m_peerId
                   << "Cannot send handshake request: not connected";
        return;
    }

    const auto& profile = core::UserProfile::instance();

    flykylin::protocol::HandshakeRequest request;
    request.set_protocol_version("1.0");
    request.set_user_id(profile.userId().toStdString());
    request.set_user_name(profile.userName().toStdString());
    request.set_timestamp(QDateTime::currentMSecsSinceEpoch());

    std::string payload;
    if (!request.SerializeToString(&payload)) {
        qCritical() << "[TcpConnection]" << m_peerId
                    << "Failed to serialize HandshakeRequest";
        m_handshakeState = HandshakeState::Failed;
        emit handshakeFailed(QStringLiteral("Failed to serialize handshake request"));
        return;
    }

    flykylin::protocol::TcpMessage tcpMessage;
    tcpMessage.set_protocol_version(1);
    tcpMessage.set_type(flykylin::protocol::TcpMessage::HANDSHAKE_REQUEST);
    tcpMessage.set_sequence(0);  // Sequence not used yet
    tcpMessage.set_payload(payload);
    tcpMessage.set_timestamp(QDateTime::currentMSecsSinceEpoch());

    QByteArray data(tcpMessage.ByteSizeLong(), Qt::Uninitialized);
    if (!tcpMessage.SerializeToArray(data.data(), data.size())) {
        qCritical() << "[TcpConnection]" << m_peerId
                    << "Failed to serialize TcpMessage for handshake request";
        m_handshakeState = HandshakeState::Failed;
        emit handshakeFailed(QStringLiteral("Failed to serialize handshake request wrapper"));
        return;
    }

    // Send as framed message: [4-byte length][protobuf payload]
    QByteArray frame;
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint32>(data.size());
    frame.append(data);

    qint64 written = m_socket->write(frame);
    if (written == -1) {
        QString error = QString("Failed to send handshake request: %1").arg(m_socket->errorString());
        qCritical() << "[TcpConnection]" << m_peerId << error;
        m_handshakeState = HandshakeState::Failed;
        emit handshakeFailed(error);
        return;
    }

    m_socket->flush();
    m_lastActivity = QDateTime::currentDateTime();

    qInfo() << "[TcpConnection]" << m_peerId
            << "Handshake request sent, size=" << data.size() << "bytes";
}

void TcpConnection::sendHandshakeResponse(bool accepted, const QString& errorMsg) {
    if (m_state != ConnectionState::Connected) {
        qWarning() << "[TcpConnection]" << m_peerId
                   << "Cannot send handshake response: not connected";
        return;
    }

    const auto& profile = core::UserProfile::instance();

    flykylin::protocol::HandshakeResponse response;
    response.set_accepted(accepted);
    response.set_user_id(profile.userId().toStdString());
    response.set_user_name(profile.userName().toStdString());
    response.set_error_message(errorMsg.toStdString());
    response.set_timestamp(QDateTime::currentMSecsSinceEpoch());

    std::string payload;
    if (!response.SerializeToString(&payload)) {
        qCritical() << "[TcpConnection]" << m_peerId
                    << "Failed to serialize HandshakeResponse";
        emit handshakeFailed(QStringLiteral("Failed to serialize handshake response"));
        return;
    }

    flykylin::protocol::TcpMessage tcpMessage;
    tcpMessage.set_protocol_version(1);
    tcpMessage.set_type(flykylin::protocol::TcpMessage::HANDSHAKE_RESPONSE);
    tcpMessage.set_sequence(0);
    tcpMessage.set_payload(payload);
    tcpMessage.set_timestamp(QDateTime::currentMSecsSinceEpoch());

    QByteArray data(tcpMessage.ByteSizeLong(), Qt::Uninitialized);
    if (!tcpMessage.SerializeToArray(data.data(), data.size())) {
        qCritical() << "[TcpConnection]" << m_peerId
                    << "Failed to serialize TcpMessage for handshake response";
        emit handshakeFailed(QStringLiteral("Failed to serialize handshake response wrapper"));
        return;
    }

    QByteArray frame;
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint32>(data.size());
    frame.append(data);

    qint64 written = m_socket->write(frame);
    if (written == -1) {
        QString error = QString("Failed to send handshake response: %1").arg(m_socket->errorString());
        qCritical() << "[TcpConnection]" << m_peerId << error;
        emit handshakeFailed(error);
        return;
    }

    m_socket->flush();
    m_lastActivity = QDateTime::currentDateTime();

    qInfo() << "[TcpConnection]" << m_peerId
            << "Handshake response sent, accepted=" << accepted;
}

void TcpConnection::handleHandshakeRequest(const QByteArray& payload) {
    flykylin::protocol::HandshakeRequest request;
    if (!request.ParseFromArray(payload.constData(), payload.size())) {
        qWarning() << "[TcpConnection]" << m_peerId
                   << "Failed to parse HandshakeRequest, payload size=" << payload.size();
        return;
    }

    // Update logical peer ID from handshake user_id when available.
    // For incoming connections, m_peerId is initially a temporary IP:port key
    // assigned by TcpServer; after handshake we prefer the stable userId.
    QString remoteUserId = QString::fromStdString(request.user_id());
    if (!remoteUserId.isEmpty()) {
        QString oldPeerId = m_peerId;
        if (remoteUserId != oldPeerId) {
            m_peerId = remoteUserId;
            emit peerIdUpdated(oldPeerId, remoteUserId);
        }
    }

    m_peerName = QString::fromStdString(request.user_name());

    qInfo() << "[TcpConnection]" << m_peerId
            << "Received handshake request from peer_name=" << m_peerName
            << "protocol=" << QString::fromStdString(request.protocol_version());

    // For now we always accept; policy checks can be added here later.
    sendHandshakeResponse(true, QString());

    if (m_handshakeState != HandshakeState::Completed) {
        m_handshakeState = HandshakeState::Completed;
        startHeartbeat();
        emit handshakeCompleted();
    }
}

void TcpConnection::handleHandshakeResponse(const QByteArray& payload) {
    flykylin::protocol::HandshakeResponse response;
    if (!response.ParseFromArray(payload.constData(), payload.size())) {
        qWarning() << "[TcpConnection]" << m_peerId
                   << "Failed to parse HandshakeResponse, payload size=" << payload.size();
        return;
    }

    m_handshakeTimer->stop();
    m_peerName = QString::fromStdString(response.user_name());

    if (!response.accepted()) {
        QString error = QString::fromStdString(response.error_message());
        if (error.isEmpty()) {
            error = QStringLiteral("Handshake rejected by peer");
        }

        qWarning() << "[TcpConnection]" << m_peerId
                   << "Handshake rejected by peer, error=" << error;

        m_handshakeState = HandshakeState::Failed;
        emit handshakeFailed(error);
        disconnectFromHost();
        return;
    }

    if (m_handshakeState == HandshakeState::Completed) {
        // Duplicate response; ignore.
        return;
    }

    m_handshakeState = HandshakeState::Completed;

    qInfo() << "[TcpConnection]" << m_peerId
            << "Handshake completed with peer_name=" << m_peerName;

    startHeartbeat();
    emit handshakeCompleted();
}

void TcpConnection::onHandshakeTimeout() {
    if (m_handshakeState == HandshakeState::Completed ||
        m_handshakeState == HandshakeState::Failed) {
        return;
    }

    QString error = QStringLiteral("Handshake timeout");
    qWarning() << "[TcpConnection]" << m_peerId << error;

    m_handshakeState = HandshakeState::Failed;
    emit handshakeFailed(error);

    // Close the connection; caller can decide whether to reconnect.
    disconnectFromHost();
}

} // namespace communication
} // namespace flykylin
