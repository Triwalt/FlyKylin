/**
 * @file MessageService.h
 * @brief Message sending/receiving service (business logic layer)
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QByteArray>
#include <QList>
#include <QMap>
#include "core/models/Message.h"
#include "core/communication/TcpConnectionManager.h"
#include "LocalEchoService.h"
#include "FileTransferService.h"

namespace flykylin {
namespace services {

/**
 * @brief Message service handling text message send/receive
 * 
 * Responsibilities:
 * - Convert UI message to Protobuf format
 * - Send text messages via TcpConnectionManager
 * - Receive and parse Protobuf messages
 * - Emit signals for UI updates
 * - Manage message history (in-memory)
 */
class MessageService : public QObject {
    Q_OBJECT
    
public:
    explicit MessageService(QObject* parent = nullptr);
    ~MessageService() override;
    
    /**
     * @brief Send text message to peer
     * @param peerId Target peer ID
     * @param content Message text content
     */
    void sendTextMessage(const QString& peerId, const QString& content);
    void sendImageMessage(const QString& peerId, const QString& filePath);
    void sendFileMessage(const QString& peerId, const QString& filePath);
    
    /**
     * @brief Get message history with a peer
     * @param peerId Peer ID
     * @return List of messages (sorted by timestamp)
     */
    QList<core::Message> getMessageHistory(const QString& peerId) const;
    
    /**
     * @brief Clear message history with a peer
     * @param peerId Peer ID
     */
    void clearHistory(const QString& peerId);
    
signals:
    /**
     * @brief Text message received from peer
     * @param message Received message object
     */
    void messageReceived(const flykylin::core::Message& message);
    
    /**
     * @brief Message sent successfully
     * @param message Sent message object (with updated status)
     */
    void messageSent(const flykylin::core::Message& message);
    
    /**
     * @brief Message send failed
     * @param message Failed message object
     * @param error Error description
     */
    void messageFailed(const flykylin::core::Message& message, const QString& error);
    
private slots:
    void onTcpMessageReceived(QString peerId, QByteArray data);
    void onTcpMessageSent(QString peerId, quint64 messageId);
    void onTcpMessageFailed(QString peerId, quint64 messageId, QString error);
    
private:
    // Protobuf conversion
    QByteArray serializeTextMessage(const core::Message& message);
    core::Message parseTextMessage(const QString& peerId, const QByteArray& data);
    
    // Message storage
    void storeMessage(const core::Message& message);
    core::Message* findPendingMessage(const QString& peerId, quint64 messageId);
    
    communication::TcpConnectionManager* m_connectionManager;
    
    // In-memory message storage: peerId -> messages
    QMap<QString, QList<core::Message>> m_messageHistory;
    
    // Pending messages (waiting for send confirmation): (peerId, messageId) -> message
    QMap<QPair<QString, quint64>, core::Message> m_pendingMessages;
    
    // Local user info
    QString m_localUserId;  ///< Local user ID (from UserProfile)
    
    // Echo service for testing
    LocalEchoService* m_echoService;  ///< Local echo service for testing
    FileTransferService* m_fileTransferService;
};

} // namespace services
} // namespace flykylin
