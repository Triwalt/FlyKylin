/**
 * @file ChatViewModel.h
 * @brief Chat window view model (MVVM pattern)
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#pragma once

#include <QObject>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QList>
#include <QHash>
#include "core/models/Message.h"
#include "core/services/MessageService.h"

// Forward declaration
namespace flykylin { namespace core { namespace services { class GroupChatManager; } } }

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
    Q_PROPERTY(QStandardItemModel* messageModel READ messageModel CONSTANT)
    Q_PROPERTY(QString currentPeerName READ getCurrentPeerName NOTIFY peerChanged)
    Q_PROPERTY(bool hasMoreHistory READ hasMoreHistory NOTIFY messagesUpdated)
    
public:
    explicit ChatViewModel(QObject* parent = nullptr);
    ~ChatViewModel() override;

    QStandardItemModel* messageModel() const { return m_messageModel; }
    
    /**
     * @brief Set current chat peer
     * @param peerId Peer ID to chat with
     * @param peerName Peer display name
     */
    void setCurrentPeer(const QString& peerId, const QString& peerName);

    /**
     * @brief Set current group chat (local pseudo group)
     * @param groupId Group identifier
     * @param groupName Group display name
     * @param memberIds Member user IDs in this group
     */
    Q_INVOKABLE void setCurrentGroup(const QString& groupId,
                                     const QString& groupName,
                                     const QStringList& memberIds);
    
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
    Q_INVOKABLE void sendMessage(const QString& content);
    Q_INVOKABLE void sendImage(const QString& filePath);
    Q_INVOKABLE void sendFile(const QString& filePath);
    Q_INVOKABLE void sendScreenshot();
    Q_INVOKABLE QString captureScreenForSelection();
    Q_INVOKABLE void sendCroppedScreenshot(const QString& fullPath,
                                           int x,
                                           int y,
                                           int width,
                                           int height);
    Q_INVOKABLE void deleteConversation(const QString& peerId);

    Q_INVOKABLE int findMessageRow(const QString& messageId) const;

    Q_INVOKABLE bool isGroupChat() const { return m_isGroupChat; }
    Q_INVOKABLE QString getCurrentGroupId() const { return m_currentGroupId; }
    Q_INVOKABLE bool hasMoreHistory() const { return m_hasMoreHistory; }

    Q_INVOKABLE bool isImageNsfw(const QString& filePath) const;

    Q_INVOKABLE void resetConversation();
    Q_INVOKABLE void loadMoreHistory();
    Q_INVOKABLE QString getOldestMessageId() const {
        return m_messages.isEmpty() ? QString() : m_messages.first().id();
    }
    
    /**
     * @brief Clear chat history with current peer
     */
    void clearHistory();
    
    /**
     * @brief Refresh message list from service
     */
    void refreshMessages();

    Q_INVOKABLE void registerGroup(const QString& groupId,
                                   const QStringList& memberIds,
                                   const QString& ownerId);
    
signals:
    /**
     * @brief Peer changed
     * @param peerId New peer ID
     * @param peerName New peer name
     */
    void peerChanged(QString peerId, QString peerName);
    
    /**
     * @brief About to rebuild message model
     * 
     * Emitted before clearing and rebuilding the message model.
     * View should save scroll position state before this happens.
     */
    void aboutToRebuildModel();
    
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
    
    /**
     * @brief Discovered a group message (used for auto-joining groups across devices)
     * @param groupId Group identifier carried by the message
     * @param fromUserId Sender user ID
     * @param toUserId Receiver user ID
     */
    void groupMessageDiscovered(const QString& groupId,
                                const QString& fromUserId,
                                const QString& toUserId);
    
private slots:
    void onMessageReceived(const flykylin::core::Message& message);
    void onMessageSent(const flykylin::core::Message& message);
    void onMessageFailed(const flykylin::core::Message& message, const QString& error);
    
private:
    void loadMessagesFromService();
    void rebuildMessageModel();
    void appendMessageToModel(const core::Message& msg);
    
    services::MessageService* m_messageService;
    
    QString m_currentPeerId;    ///< Current chat peer ID
    QString m_currentPeerName;  ///< Current chat peer name or group name
    bool m_isGroupChat{false};  ///< Whether current session is a group chat
    QString m_currentGroupId;   ///< Current group ID (for group chats)
    QStringList m_groupMembers; ///< Member user IDs in current group
    QList<core::Message> m_messages;  ///< Message list for current peer or group
    QStandardItemModel* m_messageModel; ///< Model for QML
    bool m_hasMoreHistory{false};

    // GroupChatManager is used for group metadata (singleton accessed via instance())
    // No longer using internal GroupMeta struct
};

} // namespace ui
} // namespace flykylin
