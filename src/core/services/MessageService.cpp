/**
 * @file MessageService.cpp
 * @brief Message service implementation
 * @author FlyKylin Development Team
 * @date 2024-11-20
 */

#include "MessageService.h"
#include "../config/UserProfile.h"
#include "../database/DatabaseService.h"
#include <QDebug>
#include <string>
#include "messages.pb.h"

namespace flykylin {
namespace services {

using database::DatabaseService;

MessageService::MessageService(QObject* parent)
    : QObject(parent)
    , m_connectionManager(communication::TcpConnectionManager::instance())
    , m_echoService(new LocalEchoService(this))
    , m_fileTransferService(new FileTransferService(this))
{
    // Get local user ID from UserProfile singleton
    m_localUserId = core::UserProfile::instance().userId();
    
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
    
    connect(m_fileTransferService, &FileTransferService::messageCreated,
            this, [this](const core::Message& message) {
                storeMessage(message);
                if (message.fromUserId() == m_localUserId) {
                    emit messageSent(message);
                } else {
                    emit messageReceived(message);
                }
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
    
    // Create complete message object with all required fields
    core::Message message;
    message.setId(core::Message::generateMessageId());  // Use UUID
    message.setFromUserId(m_localUserId);
    message.setToUserId(peerId);
    message.setContent(content);
    message.setTimestamp(QDateTime::currentDateTime());
    message.setStatus(core::MessageStatus::Sending);
    
    // Serialize to Protobuf
    QByteArray data = serializeTextMessage(message);
    if (data.isEmpty()) {
        qCritical() << "[MessageService] Failed to serialize message";
        message.setStatus(core::MessageStatus::Failed);
        emit messageFailed(message, "Serialization failed");
        return;
    }
    
    // Store as pending (use temporary sequence 0, will be updated in callback)
    m_pendingMessages[qMakePair(peerId, 0ULL)] = message;
    
    // Send via TCP
    m_connectionManager->sendMessage(peerId, data, 
                                    communication::MessageQueue::Priority::High);
    
    qInfo() << "[MessageService] Message queued for" << peerId 
            << "id=" << message.id() << "content:" << content.left(20) << "...";
}

void MessageService::sendGroupTextMessage(const QString& groupId,
                                          const QStringList& memberIds,
                                          const QString& content)
{
    if (groupId.trimmed().isEmpty()) {
        qWarning() << "[MessageService] Cannot send group message: empty groupId";
        return;
    }

    if (memberIds.isEmpty()) {
        qWarning() << "[MessageService] Cannot send group message: no members for" << groupId;
        return;
    }

    if (content.trimmed().isEmpty()) {
        qWarning() << "[MessageService] Cannot send empty group message";
        return;
    }

    // Generate a single logical message ID for this group message. All
    // per-recipient TCP sends for this logical message will share the same
    // ID so that the chat history and UI only contain one entry per
    // outbound group message on the sender side.
    const QString groupMessageId = core::Message::generateMessageId();

    for (const QString& peerId : memberIds) {
        if (peerId == LocalEchoService::getEchoBotId()) {
            continue;
        }

        qInfo() << "[MessageService] Sending group message to" << peerId << "group" << groupId;

        core::Message message;
        message.setId(groupMessageId);
        message.setFromUserId(m_localUserId);
        message.setToUserId(peerId);
        message.setContent(content);
        message.setTimestamp(QDateTime::currentDateTime());
        message.setStatus(core::MessageStatus::Sending);
        message.setIsGroup(true);
        message.setGroupId(groupId);

        QByteArray data = serializeTextMessage(message);
        if (data.isEmpty()) {
            qCritical() << "[MessageService] Failed to serialize group message for" << peerId;
            message.setStatus(core::MessageStatus::Failed);
            emit messageFailed(message, QStringLiteral("Serialization failed"));
            continue;
        }

        m_pendingMessages[qMakePair(peerId, 0ULL)] = message;

        m_connectionManager->sendMessage(peerId,
                                         data,
                                         communication::MessageQueue::Priority::High);

        qInfo() << "[MessageService] Group message queued for" << peerId
                << "group" << groupId << "id=" << message.id()
                << "content:" << content.left(20) << "...";
    }
}

void MessageService::relayGroupTextMessage(const core::Message& originalMessage,
                                           const QStringList& relayTargets)
{
    if (!originalMessage.isGroup() || originalMessage.groupId().trimmed().isEmpty()) {
        qWarning() << "[MessageService] Cannot relay non-group or empty-groupId message";
        return;
    }

    if (relayTargets.isEmpty()) {
        return;
    }

    for (const QString& peerId : relayTargets) {
        if (peerId.isEmpty()) {
            continue;
        }
        if (peerId == LocalEchoService::getEchoBotId()) {
            continue;
        }

        core::Message message = originalMessage;
        message.setToUserId(peerId);
        message.setStatus(core::MessageStatus::Sending);

        QByteArray data = serializeTextMessage(message);
        if (data.isEmpty()) {
            qCritical() << "[MessageService] Failed to serialize relayed group message for"
                        << peerId;
            continue;
        }

        m_connectionManager->sendMessage(peerId,
                                         data,
                                         communication::MessageQueue::Priority::High);

        qInfo() << "[MessageService] Relayed group message" << message.id() << "for group"
                << message.groupId() << "to" << peerId;
    }
}

void MessageService::sendGroupImageMessage(const QString& groupId,
                                           const QStringList& memberIds,
                                           const QString& filePath)
{
    if (groupId.trimmed().isEmpty()) {
        qWarning() << "[MessageService] Cannot send group image: empty groupId";
        return;
    }

    if (memberIds.isEmpty()) {
        qWarning() << "[MessageService] Cannot send group image: no members for" << groupId;
        return;
    }

    if (filePath.trimmed().isEmpty()) {
        qWarning() << "[MessageService] Cannot send group image: empty file path";
        return;
    }

    const QString groupMessageId = core::Message::generateMessageId();

    for (const QString& peerId : memberIds) {
        if (peerId.isEmpty() || peerId == LocalEchoService::getEchoBotId()) {
            continue;
        }

        qInfo() << "[MessageService] Sending group image to" << peerId << "group" << groupId
                << "path=" << filePath;

        if (!m_fileTransferService) {
            qWarning() << "[MessageService] FileTransferService is null";
            return;
        }

        m_fileTransferService->sendImage(peerId,
                                         filePath,
                                         true,   // isGroup
                                         groupId,
                                         groupMessageId);
    }
}

void MessageService::sendGroupFileMessage(const QString& groupId,
                                          const QStringList& memberIds,
                                          const QString& filePath)
{
    if (groupId.trimmed().isEmpty()) {
        qWarning() << "[MessageService] Cannot send group file: empty groupId";
        return;
    }

    if (memberIds.isEmpty()) {
        qWarning() << "[MessageService] Cannot send group file: no members for" << groupId;
        return;
    }

    if (filePath.trimmed().isEmpty()) {
        qWarning() << "[MessageService] Cannot send group file: empty file path";
        return;
    }

    const QString groupMessageId = core::Message::generateMessageId();

    for (const QString& peerId : memberIds) {
        if (peerId.isEmpty() || peerId == LocalEchoService::getEchoBotId()) {
            continue;
        }

        qInfo() << "[MessageService] Sending group file to" << peerId << "group" << groupId
                << "path=" << filePath;

        if (!m_fileTransferService) {
            qWarning() << "[MessageService] FileTransferService is null";
            return;
        }

        m_fileTransferService->sendFile(peerId,
                                        filePath,
                                        true,   // isGroup
                                        groupId,
                                        groupMessageId);
    }
}

void MessageService::relayGroupFileMessage(const core::Message& originalMessage,
                                           const QStringList& relayTargets)
{
    if (!originalMessage.isGroup() || originalMessage.groupId().trimmed().isEmpty()) {
        qWarning() << "[MessageService] Cannot relay non-group or empty-groupId file message";
        return;
    }

    if (relayTargets.isEmpty()) {
        return;
    }

    const QString filePath = originalMessage.attachmentLocalPath();
    if (filePath.trimmed().isEmpty()) {
        qWarning() << "[MessageService] Cannot relay group file: empty attachment path for"
                   << originalMessage.id();
        return;
    }

    if (!m_fileTransferService) {
        qWarning() << "[MessageService] FileTransferService is null, cannot relay group file";
        return;
    }

    const QString groupId = originalMessage.groupId();
    const QString logicalId = originalMessage.id();
    const bool asImage = (originalMessage.kind() == core::MessageKind::Image);

    for (const QString& peerId : relayTargets) {
        if (peerId.isEmpty() || peerId == LocalEchoService::getEchoBotId()) {
            continue;
        }

        qInfo() << "[MessageService] Relaying group" << (asImage ? "image" : "file")
                << logicalId << "for group" << groupId << "to" << peerId;

        if (asImage) {
            m_fileTransferService->sendImage(peerId,
                                             filePath,
                                             true,   // isGroup
                                             groupId,
                                             logicalId);
        } else {
            m_fileTransferService->sendFile(peerId,
                                            filePath,
                                            true,   // isGroup
                                            groupId,
                                            logicalId);
        }
    }
}

void MessageService::sendImageMessage(const QString& peerId, const QString& filePath) {
    if (peerId == LocalEchoService::getEchoBotId()) {
        qInfo() << "[MessageService] File/image sending to Echo Bot is not supported";
        return;
    }

    if (!m_fileTransferService) {
        qWarning() << "[MessageService] FileTransferService is null";
        return;
    }

    qInfo() << "[MessageService] Sending image to" << peerId << "path=" << filePath;
    m_fileTransferService->sendImage(peerId, filePath);
}

void MessageService::sendFileMessage(const QString& peerId, const QString& filePath) {
    if (peerId == LocalEchoService::getEchoBotId()) {
        qInfo() << "[MessageService] File/image sending to Echo Bot is not supported";
        return;
    }

    if (!m_fileTransferService) {
        qWarning() << "[MessageService] FileTransferService is null";
        return;
    }

    qInfo() << "[MessageService] Sending file to" << peerId << "path=" << filePath;
    m_fileTransferService->sendFile(peerId, filePath);
}

QList<core::Message> MessageService::getMessageHistory(const QString& peerId) const {
    // Prefer persistent history from database; fall back to in-memory cache
    QList<core::Message> fromDb = DatabaseService::instance()->loadMessages(m_localUserId, peerId);
    if (!fromDb.isEmpty()) {
        return fromDb;
    }

    return m_messageHistory.value(peerId, QList<core::Message>());
}

QList<core::Message> MessageService::getLatestMessages(const QString& peerId, int limit) const {
    if (peerId.isEmpty()) {
        return QList<core::Message>();
    }

    return DatabaseService::instance()->loadLatestMessages(m_localUserId, peerId, limit);
}

QList<core::Message> MessageService::getMessagesBefore(const QString& peerId,
                                                       const QDateTime& before,
                                                       int limit) const
{
    if (peerId.isEmpty() || !before.isValid()) {
        return QList<core::Message>();
    }

    return DatabaseService::instance()->loadMessagesBefore(
        m_localUserId,
        peerId,
        before.toMSecsSinceEpoch(),
        limit);
}

QList<core::Message> MessageService::getLatestGroupMessages(const QString& groupId,
                                                            int limit) const
{
    if (groupId.isEmpty()) {
        return QList<core::Message>();
    }

    return DatabaseService::instance()->loadLatestGroupMessages(m_localUserId, groupId, limit);
}

QList<core::Message> MessageService::getGroupMessagesBefore(const QString& groupId,
                                                            const QDateTime& before,
                                                            int limit) const
{
    if (groupId.isEmpty() || !before.isValid()) {
        return QList<core::Message>();
    }

    return DatabaseService::instance()->loadGroupMessagesBefore(
        m_localUserId,
        groupId,
        before.toMSecsSinceEpoch(),
        limit);
}

void MessageService::clearHistory(const QString& peerId) {
    m_messageHistory.remove(peerId);
    DatabaseService::instance()->clearHistory(m_localUserId, peerId);
    qInfo() << "[MessageService] Cleared history for" << peerId;
}

void MessageService::onTcpMessageReceived(QString peerId, QByteArray data) {
    qInfo() << "[MessageService] Received TCP message from" << peerId 
            << "size=" << data.size();
    
    flykylin::protocol::TcpMessage tcpMsg;
    if (!tcpMsg.ParseFromArray(data.data(), data.size())) {
        qCritical() << "[MessageService] Failed to parse TcpMessage";
        return;
    }

    qInfo() << "[MessageService] TcpMessage type=" << tcpMsg.type();

    if (tcpMsg.type() != flykylin::protocol::TcpMessage::TEXT) {
        return;
    }

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
            << "isGroup=" << message.isGroup() << "groupId=" << message.groupId()
            << "content:" << message.content().left(20) << "...";
}

void MessageService::onTcpMessageSent(QString peerId, quint64 messageId) {
    qDebug() << "[MessageService] TCP message sent to" << peerId 
             << "id=" << messageId;
    
    // Find pending message
    core::Message* msg = findPendingMessage(peerId, messageId);
    if (msg) {
        msg->setStatus(core::MessageStatus::Sent);
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
        msg->setStatus(core::MessageStatus::Failed);
        storeMessage(*msg);
        emit messageFailed(*msg, error);
        
        // Remove from pending
        m_pendingMessages.remove(qMakePair(peerId, messageId));
    }
}

QByteArray MessageService::serializeTextMessage(const core::Message& message) {
    // Create Protobuf TextMessage with complete fields
    flykylin::protocol::TextMessage textMsg;
    textMsg.set_message_id(message.id().toStdString());
    textMsg.set_from_user_id(message.fromUserId().toStdString());
    textMsg.set_to_user_id(message.toUserId().toStdString());
    textMsg.set_content(message.content().toStdString());
    textMsg.set_timestamp(message.timestamp().toMSecsSinceEpoch());
    textMsg.set_is_group(message.isGroup());
    if (message.isGroup() && !message.groupId().isEmpty()) {
        textMsg.add_group_ids(message.groupId().toStdString());
    }

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

    qDebug() << "[MessageService] Serialized message id=" << message.id()
             << "size=" << data.size() << "bytes";

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

    // Fill Message object with complete fields
    message.setId(QString::fromStdString(textMsg.message_id()));
    message.setFromUserId(QString::fromStdString(textMsg.from_user_id()));
    message.setToUserId(QString::fromStdString(textMsg.to_user_id()));
    message.setContent(QString::fromStdString(textMsg.content()));
    message.setTimestamp(QDateTime::fromMSecsSinceEpoch(textMsg.timestamp()));
    message.setStatus(core::MessageStatus::Delivered);

    message.setIsGroup(textMsg.is_group());
    if (textMsg.group_ids_size() > 0) {
        message.setGroupId(QString::fromStdString(textMsg.group_ids(0)));
    }

    qInfo() << "[MessageService] Parsed message id=" << message.id()
            << "from=" << message.fromUserId() << "to=" << message.toUserId()
            << "isGroup=" << message.isGroup() << "groupId=" << message.groupId();

    return message;
}

void MessageService::storeMessage(const core::Message& message) {
    QString peerId;
    if (message.isGroup() && !message.groupId().isEmpty()) {
        peerId = message.groupId();
    } else {
        peerId = (message.fromUserId() == m_localUserId)
                     ? message.toUserId()
                     : message.fromUserId();
    }

    m_messageHistory[peerId].append(message);
    DatabaseService::instance()->appendMessage(message, m_localUserId);
    DatabaseService::instance()->touchSession(m_localUserId,
                                              peerId,
                                              message.timestamp().toMSecsSinceEpoch());

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
        // Promote the pending entry to use the real TCP sequence number as
        // map key, but keep the application-level message ID unchanged so
        // that all per-recipient sends for a logical group message share
        // the same ID.
        core::Message msg = m_pendingMessages.take(tempKey);
        m_pendingMessages[key] = msg;
        return &m_pendingMessages[key];
    }
    
    qWarning() << "[MessageService] Pending message not found:" << peerId << messageId;
    return nullptr;
}

} // namespace services
} // namespace flykylin
