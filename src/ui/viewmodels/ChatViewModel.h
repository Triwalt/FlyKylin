/**
 * @file ChatViewModel.h
 * @brief Chat window view model (MVVM pattern)
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include "core/models/Message.h"
#include "core/services/MessageService.h"

namespace flykylin {
namespace ui {

/**
 * @brief Chat window view model following MVVM pattern
 * 
 * Responsibilities:
 * - Manage chat message list for current peer
 * - Handle send message requests from view
 * - Update view when new messages arrive
 * - Provide data for UI rendering
 */
class ChatViewModel : public QObject {
    Q_OBJECT
    
public:
    explicit ChatViewModel(QObject* parent = nullptr);
    ~ChatViewModel() override;
    
    /**
     * @brief Set current chat peer
     * @param peerId Peer ID to chat with
     * @param peerName Peer display name
     */
    void setCurrentPeer(const QString& peerId, const QString& peerName);
    
    /**
     * @brief Get current peer ID
     */
    QString getCurrentPeerId() const { return m_currentPeerId; }
    
    /**
     * @brief Get current peer name
     */
    QString getCurrentPeerName() const { return m_currentPeerName; }
    
    /**
     * @brief Get message list for current peer
     */
    QList<core::Message> getMessages() const { return m_messages; }
    
    /**
     * @brief Send text message to current peer
     * @param content Message content
     */
    void sendMessage(const QString& content);
    
    /**
     * @brief Clear chat history with current peer
     */
    void clearHistory();
    
    /**
     * @brief Refresh message list from service
     */
    void refreshMessages();
    
signals:
    /**
     * @brief Peer changed
     * @param peerId New peer ID
     * @param peerName New peer name
     */
    void peerChanged(QString peerId, QString peerName);
    
    /**
     * @brief Message list updated
     * 
     * View should re-render the message list
     */
    void messagesUpdated();
    
    /**
     * @brief New message received
     * @param message Received message
     * 
     * View should scroll to bottom and show notification
     */
    void messageReceived(const flykylin::core::Message& message);
    
    /**
     * @brief Message sent successfully
     * @param message Sent message
     */
    void messageSent(const flykylin::core::Message& message);
    
    /**
     * @brief Message send failed
     * @param message Failed message
     * @param error Error description
     */
    void messageFailed(const flykylin::core::Message& message, QString error);
    
private slots:
    void onMessageReceived(const flykylin::core::Message& message);
    void onMessageSent(const flykylin::core::Message& message);
    void onMessageFailed(const flykylin::core::Message& message, const QString& error);
    
private:
    void loadMessagesFromService();
    
    services::MessageService* m_messageService;
    
    QString m_currentPeerId;    ///< Current chat peer ID
    QString m_currentPeerName;  ///< Current chat peer name
    QList<core::Message> m_messages;  ///< Message list for current peer
};

} // namespace ui
} // namespace flykylin
