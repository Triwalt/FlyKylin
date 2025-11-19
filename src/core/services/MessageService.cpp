/**
 * @file MessageService.cpp
 * @brief Message service implementation
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#include "MessageService.h"
#include <QDebug>
#include <string>

namespace flykylin {
namespace services {

MessageService::MessageService(QObject* parent)
    : QObject(parent)
    , m_connectionManager(communication::TcpConnectionManager::instance())
    , m_echoService(new LocalEchoService(this))
{
    // TODO: Get local user ID from UserProfile singleton when implemented
    // For now, use a temporary placeholder
    m_localUserId = "local_user";
    
    // Connect TCP signals
    connect(m_connectionManager, &communication::TcpConnectionManager::messageReceived,
            this, &MessageService::onTcpMessageReceived);
    connect(m_connectionManager, &communication::TcpConnectionManager::messageSent,
            this, &MessageService::onTcpMessageSent);
    connect(m_connectionManager, &communication::TcpConnectionManager::messageFailed,
            this, &MessageService::onTcpMessageFailed);
    
    // Connect Echo service signals
    connect(m_echoService, &LocalEchoService::messageReceived,
            this, [this](const core::Message& message) {
                // Store and emit the echo message
                storeMessage(message);
                emit messageReceived(message);
                qInfo() << "[MessageService] Echo message received:" << message.content().left(30);
            });
    
    qInfo() << "[MessageService] Initialized for user" << m_localUserId 
            << "with Echo Bot support";
}

MessageService::~MessageService() {
    qInfo() << "[MessageService] Destroyed";
}

void MessageService::sendTextMessage(const QString& peerId, const QString& content) {
    if (content.trimmed().isEmpty()) {
        qWarning() << "[MessageService] Cannot send empty message";
        return;
    }
    
    qInfo() << "[MessageService] Sending message to" << peerId;
    
    // Check if sending to Echo Bot (local testing)
    if (peerId == LocalEchoService::getEchoBotId()) {
        qInfo() << "[MessageService] Routing message to Echo Bot for local testing";
        m_echoService->sendToEchoBot(content);
        return;
    }
    
    // Create message object
    core::Message message;
    message.setId(QString::number(QDateTime::currentMSecsSinceEpoch()));
    // Note: core::Message doesn't have senderId/receiverId setters yet
    message.setContent(content);
    message.setTimestamp(QDateTime::currentDateTime());
    
    // Serialize to Protobuf
    QByteArray data = serializeTextMessage(content);
    
    // Store as pending
    // Note: We'll get the actual TCP message ID from onTcpMessageSent callback
    // For now, store with temporary ID
    m_pendingMessages[qMakePair(peerId, 0ULL)] = message;
    
    // Send via TCP
    m_connectionManager->sendMessage(peerId, data, 
                                    communication::MessageQueue::Priority::High);
    
    qInfo() << "[MessageService] Sending message to" << peerId 
            << "content:" << content.left(20) << "...";
}

QList<core::Message> MessageService::getMessageHistory(const QString& peerId) const {
    return m_messageHistory.value(peerId, QList<core::Message>());
}

void MessageService::clearHistory(const QString& peerId) {
    m_messageHistory.remove(peerId);
    qInfo() << "[MessageService] Cleared history for" << peerId;
}

void MessageService::onTcpMessageReceived(QString peerId, QByteArray data) {
    qDebug() << "[MessageService] Received TCP message from" << peerId 
             << "size=" << data.size();
    
    // Parse Protobuf message
    core::Message message = parseTextMessage(peerId, data);
    
    if (message.content().isEmpty()) {
        qWarning() << "[MessageService] Failed to parse message from" << peerId;
        return;
    }
    
    // Store in history
    storeMessage(message);
    
    // Emit signal for UI
    emit messageReceived(message);
    
    qInfo() << "[MessageService] Message received from" << peerId 
            << "content:" << message.content().left(20) << "...";
}

void MessageService::onTcpMessageSent(QString peerId, quint64 messageId) {
    qDebug() << "[MessageService] TCP message sent to" << peerId 
             << "id=" << messageId;
    
    // Find pending message
    core::Message* msg = findPendingMessage(peerId, messageId);
    if (msg) {
        // TODO: Add status field to Message class
        // msg->setStatus(core::Message::Sent);
        storeMessage(*msg);
        emit messageSent(*msg);
        
        // Remove from pending
        m_pendingMessages.remove(qMakePair(peerId, messageId));
    }
}

void MessageService::onTcpMessageFailed(QString peerId, quint64 messageId, QString error) {
    qWarning() << "[MessageService] TCP message failed to" << peerId 
               << "id=" << messageId << "error:" << error;
    
    // Find pending message
    core::Message* msg = findPendingMessage(peerId, messageId);
    if (msg) {
        // TODO: Add status field to Message class
        // msg->setStatus(core::Message::Failed);
        storeMessage(*msg);
        emit messageFailed(*msg, error);
        
        // Remove from pending
        m_pendingMessages.remove(qMakePair(peerId, messageId));
    }
}

QByteArray MessageService::serializeTextMessage(const QString& content) {
    // Create Protobuf TextMessage
    flykylin::protocol::TextMessage textMsg;
    textMsg.set_content(content.toStdString());
    textMsg.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    
    // Serialize TextMessage to payload
    std::string payload;
    if (!textMsg.SerializeToString(&payload)) {
        qCritical() << "[MessageService] Failed to serialize TextMessage";
        return QByteArray();
    }
    
    // Wrap in TcpMessage
    flykylin::protocol::TcpMessage tcpMsg;
    tcpMsg.set_protocol_version(1);
    tcpMsg.set_type(flykylin::protocol::TcpMessage::TEXT);
    tcpMsg.set_sequence(0);  // Will be set by TcpConnection
    tcpMsg.set_payload(payload);
    tcpMsg.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    
    // Serialize TcpMessage
    QByteArray data(tcpMsg.ByteSizeLong(), Qt::Uninitialized);
    if (!tcpMsg.SerializeToArray(data.data(), data.size())) {
        qCritical() << "[MessageService] Failed to serialize TcpMessage";
        return QByteArray();
    }
    
    return data;
}

core::Message MessageService::parseTextMessage(const QString& peerId, const QByteArray& data) {
    core::Message message;
    
    // Parse TcpMessage
    flykylin::protocol::TcpMessage tcpMsg;
    if (!tcpMsg.ParseFromArray(data.data(), data.size())) {
        qCritical() << "[MessageService] Failed to parse TcpMessage";
        return message;
    }
    
    // Check message type
    if (tcpMsg.type() != flykylin::protocol::TcpMessage::TEXT) {
        qWarning() << "[MessageService] Unexpected message type:" << tcpMsg.type();
        return message;
    }
    
    // Parse TextMessage from payload
    flykylin::protocol::TextMessage textMsg;
    if (!textMsg.ParseFromString(tcpMsg.payload())) {
        qCritical() << "[MessageService] Failed to parse TextMessage from payload";
        return message;
    }
    
    // Fill Message object
    message.setId(QString::number(tcpMsg.sequence()));
    message.setFromUserId(peerId);
    message.setToUserId(m_localUserId);
    message.setContent(QString::fromStdString(textMsg.content()));
    message.setTimestamp(QDateTime::fromMSecsSinceEpoch(textMsg.timestamp()));
    // TODO: Add status field to Message class
    // message.setStatus(core::Message::Received);
    
    return message;
}

void MessageService::storeMessage(const core::Message& message) {
    QString peerId = (message.fromUserId() == m_localUserId) 
                     ? message.toUserId() 
                     : message.fromUserId();
    
    m_messageHistory[peerId].append(message);
    
    qDebug() << "[MessageService] Stored message, history size for" << peerId 
             << "=" << m_messageHistory[peerId].size();
}

core::Message* MessageService::findPendingMessage(const QString& peerId, quint64 messageId) {
    auto key = qMakePair(peerId, messageId);
    
    if (m_pendingMessages.contains(key)) {
        return &m_pendingMessages[key];
    }
    
    // Also try with temporary ID (0)
    auto tempKey = qMakePair(peerId, 0ULL);
    if (m_pendingMessages.contains(tempKey)) {
        // Update with real message ID
        core::Message msg = m_pendingMessages.take(tempKey);
        msg.setId(QString::number(messageId));
        m_pendingMessages[key] = msg;
        return &m_pendingMessages[key];
    }
    
    qWarning() << "[MessageService] Pending message not found:" << peerId << messageId;
    return nullptr;
}

} // namespace services
} // namespace flykylin
