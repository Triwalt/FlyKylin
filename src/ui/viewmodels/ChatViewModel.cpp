/**
 * @file ChatViewModel.cpp
 * @brief Chat window view model implementation
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#include "ChatViewModel.h"
#include <QDebug>
#include <QDateTime>

namespace flykylin {
namespace ui {

ChatViewModel::ChatViewModel(QObject* parent)
    : QObject(parent)
    , m_messageService(new services::MessageService(this))
{
    // Connect MessageService signals
    connect(m_messageService, &services::MessageService::messageReceived,
            this, &ChatViewModel::onMessageReceived);
    connect(m_messageService, &services::MessageService::messageSent,
            this, &ChatViewModel::onMessageSent);
    connect(m_messageService, &services::MessageService::messageFailed,
            this, &ChatViewModel::onMessageFailed);
    
    qInfo() << "[ChatViewModel] Created";
}

ChatViewModel::~ChatViewModel() {
    qInfo() << "[ChatViewModel] Destroyed";
}

void ChatViewModel::setCurrentPeer(const QString& peerId, const QString& peerName) {
    if (m_currentPeerId == peerId) {
        return;  // Already chatting with this peer
    }
    
    qInfo() << "[ChatViewModel] Switching to peer" << peerId << "(" << peerName << ")";
    
    m_currentPeerId = peerId;
    m_currentPeerName = peerName;
    
    // Load message history for this peer
    loadMessagesFromService();
    
    emit peerChanged(peerId, peerName);
}

void ChatViewModel::sendMessage(const QString& content) {
    if (m_currentPeerId.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send message: no peer selected";
        return;
    }
    
    if (content.trimmed().isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send empty message";
        return;
    }
    
    qInfo() << "[ChatViewModel] Sending message to" << m_currentPeerId;
    
    // Create message object immediately for UI display
    core::Message message;
    message.setId(QString::number(QDateTime::currentMSecsSinceEpoch()));
    message.setFromUserId("local_user"); // TODO: Get from UserProfile
    message.setToUserId(m_currentPeerId);
    message.setContent(content);
    message.setTimestamp(QDateTime::currentDateTime());
    
    // Add to local message list immediately
    m_messages.append(message);
    emit messagesUpdated();
    emit messageSent(message);
    
    // Send via service (async)
    m_messageService->sendTextMessage(m_currentPeerId, content);
}

void ChatViewModel::clearHistory() {
    if (m_currentPeerId.isEmpty()) {
        return;
    }
    
    qInfo() << "[ChatViewModel] Clearing history for" << m_currentPeerId;
    
    m_messageService->clearHistory(m_currentPeerId);
    m_messages.clear();
    
    emit messagesUpdated();
}

void ChatViewModel::refreshMessages() {
    if (m_currentPeerId.isEmpty()) {
        return;
    }
    
    qDebug() << "[ChatViewModel] Refreshing messages for" << m_currentPeerId;
    loadMessagesFromService();
}

void ChatViewModel::onMessageReceived(const flykylin::core::Message& message) {
    // Check if message is for current peer
    QString peerId = message.fromUserId();
    
    if (peerId != m_currentPeerId) {
        qDebug() << "[ChatViewModel] Received message from" << peerId 
                 << "but current peer is" << m_currentPeerId << ", ignoring";
        return;
    }
    
    qInfo() << "[ChatViewModel] Message received from" << peerId;
    
    // Add to message list
    m_messages.append(message);
    
    // Notify view
    emit messageReceived(message);
    emit messagesUpdated();
}

void ChatViewModel::onMessageSent(const flykylin::core::Message& message) {
    // Check if message is for current peer
    QString peerId = message.toUserId();
    
    if (peerId != m_currentPeerId) {
        return;
    }
    
    qInfo() << "[ChatViewModel] Message sent to" << peerId;
    
    // Update message status in list
    for (auto& msg : m_messages) {
        if (msg.id() == message.id()) {
            msg = message;  // Update with sent status
            break;
        }
    }
    
    // If not found, add to list (shouldn't happen normally)
    if (std::none_of(m_messages.begin(), m_messages.end(), 
                     [&message](const core::Message& msg) {
                         return msg.id() == message.id();
                     })) {
        m_messages.append(message);
    }
    
    // Notify view
    emit messageSent(message);
    emit messagesUpdated();
}

void ChatViewModel::onMessageFailed(const flykylin::core::Message& message, const QString& error) {
    // Check if message is for current peer
    QString peerId = message.toUserId();
    
    if (peerId != m_currentPeerId) {
        return;
    }
    
    qWarning() << "[ChatViewModel] Message failed to" << peerId << "error:" << error;
    
    // Update message status in list
    for (auto& msg : m_messages) {
        if (msg.id() == message.id()) {
            msg = message;  // Update with failed status
            break;
        }
    }
    
    // Notify view
    emit messageFailed(message, error);
    emit messagesUpdated();
}

void ChatViewModel::loadMessagesFromService() {
    m_messages = m_messageService->getMessageHistory(m_currentPeerId);
    
    qInfo() << "[ChatViewModel] Loaded" << m_messages.size() << "messages for" << m_currentPeerId;
    
    emit messagesUpdated();
}

} // namespace ui
} // namespace flykylin
