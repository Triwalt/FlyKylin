/**
 * @file ChatViewModel.cpp
 * @brief Chat window view model implementation
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#include "ChatViewModel.h"
#include "../../core/models/Message.h"
#include "../../core/services/MessageService.h"
#include "../../core/config/UserProfile.h"
#include <QDateTime>
#include <QDebug>
#include <QUrl>

namespace flykylin {
namespace ui {

ChatViewModel::ChatViewModel(QObject* parent)
    : QObject(parent)
    , m_messageService(new services::MessageService(this))
    , m_messageModel(new QStandardItemModel(this))
{
    // Set role names
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole] = "content";
    roles[Qt::UserRole + 1] = "isMine";
    roles[Qt::UserRole + 2] = "timestamp";
    roles[Qt::UserRole + 3] = "status";
    roles[Qt::UserRole + 4] = "senderName";
    roles[Qt::UserRole + 5] = "kind";
    roles[Qt::UserRole + 6] = "attachmentPath";
    roles[Qt::UserRole + 7] = "attachmentName";
    m_messageModel->setItemRoleNames(roles);

    // Connect MessageService signals
    connect(m_messageService, &services::MessageService::messageReceived,
            this, &ChatViewModel::onMessageReceived);
    connect(m_messageService, &services::MessageService::messageSent,
            this, &ChatViewModel::onMessageSent);
    connect(m_messageService, &services::MessageService::messageFailed,
            this, &ChatViewModel::onMessageFailed);
    
    qInfo() << "[ChatViewModel] Created";
}

void ChatViewModel::rebuildMessageModel() {
    m_messageModel->clear();
    
    QString localUserId = core::UserProfile::instance().userId();
    
    for (const auto& msg : m_messages) {
        auto* item = new QStandardItem();
        item->setData(msg.content(), Qt::UserRole);
        item->setData(msg.fromUserId() == localUserId, Qt::UserRole + 1);
        item->setData(msg.timestamp().toString("HH:mm:ss"), Qt::UserRole + 2);
        item->setData(QVariant::fromValue(msg.status()), Qt::UserRole + 3);
        item->setData(msg.fromUserId(), Qt::UserRole + 4); // Simplified sender name

        QString kindStr = QStringLiteral("text");
        switch (msg.kind()) {
        case core::MessageKind::Image:
            kindStr = QStringLiteral("image");
            break;
        case core::MessageKind::File:
            kindStr = QStringLiteral("file");
            break;
        default:
            break;
        }

        item->setData(kindStr, Qt::UserRole + 5);
        item->setData(msg.attachmentLocalPath(), Qt::UserRole + 6);
        item->setData(msg.attachmentName(), Qt::UserRole + 7);
        m_messageModel->appendRow(item);
    }
}

ChatViewModel::~ChatViewModel() {
    qInfo() << "[ChatViewModel] Destroyed";
}

void ChatViewModel::setCurrentPeer(const QString& peerId, const QString& peerName) {
    if (m_currentPeerId == peerId) {
        return;  // Already chatting with this peer
    }
    
    qInfo() << "[ChatViewModel] Switching to peer" << peerId << "(" << peerName << ")";
    
    m_isGroupChat = false;
    m_currentGroupId.clear();
    m_groupMembers.clear();
    m_currentPeerId = peerId;
    m_currentPeerName = peerName;
    
    // Load message history for this peer
    loadMessagesFromService();
    
    emit peerChanged(peerId, peerName);
}

void ChatViewModel::setCurrentGroup(const QString& groupId,
                                    const QString& groupName,
                                    const QStringList& memberIds) {
    if (groupId.isEmpty() || memberIds.isEmpty()) {
        qWarning() << "[ChatViewModel] setCurrentGroup: invalid group or empty members";
        return;
    }

    qInfo() << "[ChatViewModel] Switching to group" << groupId << "(" << groupName << ")";

    m_isGroupChat = true;
    m_currentGroupId = groupId;
    m_groupMembers = memberIds;
    m_currentPeerId.clear();
    m_currentPeerName = groupName;

    m_messages.clear();

    for (const auto& memberId : m_groupMembers) {
        const QList<core::Message> history = m_messageService->getMessageHistory(memberId);
        for (const auto& msg : history) {
            m_messages.append(msg);
        }
    }

    std::sort(m_messages.begin(), m_messages.end(),
              [](const core::Message& a, const core::Message& b) {
                  return a.timestamp() < b.timestamp();
              });

    rebuildMessageModel();
    emit messagesUpdated();
    emit peerChanged(groupId, groupName);
}

void ChatViewModel::sendMessage(const QString& content) {
    if (!m_isGroupChat && m_currentPeerId.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send message: no peer selected";
        return;
    }

    if (m_isGroupChat && m_groupMembers.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send message: group has no members";
        return;
    }

    if (content.trimmed().isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send empty message";
        return;
    }
    
    if (!m_isGroupChat) {
        qInfo() << "[ChatViewModel] Sending message to" << m_currentPeerId;
        m_messageService->sendTextMessage(m_currentPeerId, content);
    } else {
        qInfo() << "[ChatViewModel] Sending group message to members of" << m_currentGroupId;
        for (const auto& memberId : m_groupMembers) {
            m_messageService->sendTextMessage(memberId, content);
        }
    }
}

void ChatViewModel::sendImage(const QString& filePath) {
    if (!m_isGroupChat && m_currentPeerId.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send image: no peer selected";
        return;
    }

    if (m_isGroupChat && m_groupMembers.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send image: group has no members";
        return;
    }

    if (filePath.isEmpty()) {
        return;
    }

    QString normalizedPath = filePath;
    if (filePath.startsWith("file:")) {
        const QUrl url(filePath);
        const QString local = url.toLocalFile();
        if (!local.isEmpty()) {
            normalizedPath = local;
        }
    }

    if (!m_isGroupChat) {
        qInfo() << "[ChatViewModel] Sending image to" << m_currentPeerId << "path=" << normalizedPath;
        m_messageService->sendImageMessage(m_currentPeerId, normalizedPath);
    } else {
        qInfo() << "[ChatViewModel] Sending group image to members of" << m_currentGroupId
                << "path=" << normalizedPath;
        for (const auto& memberId : m_groupMembers) {
            m_messageService->sendImageMessage(memberId, normalizedPath);
        }
    }
}

void ChatViewModel::sendFile(const QString& filePath) {
    if (!m_isGroupChat && m_currentPeerId.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send file: no peer selected";
        return;
    }

    if (m_isGroupChat && m_groupMembers.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send file: group has no members";
        return;
    }

    if (filePath.isEmpty()) {
        return;
    }

    QString normalizedPath = filePath;
    if (filePath.startsWith("file:")) {
        const QUrl url(filePath);
        const QString local = url.toLocalFile();
        if (!local.isEmpty()) {
            normalizedPath = local;
        }
    }

    if (!m_isGroupChat) {
        qInfo() << "[ChatViewModel] Sending file to" << m_currentPeerId << "path=" << normalizedPath;
        m_messageService->sendFileMessage(m_currentPeerId, normalizedPath);
    } else {
        qInfo() << "[ChatViewModel] Sending group file to members of" << m_currentGroupId
                << "path=" << normalizedPath;
        for (const auto& memberId : m_groupMembers) {
            m_messageService->sendFileMessage(memberId, normalizedPath);
        }
    }
}

void ChatViewModel::clearHistory() {
    if (m_currentPeerId.isEmpty()) {
        return;
    }
    
    qInfo() << "[ChatViewModel] Clearing history for" << m_currentPeerId;
    
    m_messageService->clearHistory(m_currentPeerId);
    m_messages.clear();
    rebuildMessageModel();
    
    emit messagesUpdated();
}

void ChatViewModel::refreshMessages() {
    if (m_isGroupChat) {
        if (m_currentGroupId.isEmpty() || m_groupMembers.isEmpty()) {
            return;
        }
        qDebug() << "[ChatViewModel] Refreshing messages for group" << m_currentGroupId;
        setCurrentGroup(m_currentGroupId, m_currentPeerName, m_groupMembers);
    } else {
        if (m_currentPeerId.isEmpty()) {
            return;
        }
        qDebug() << "[ChatViewModel] Refreshing messages for" << m_currentPeerId;
        loadMessagesFromService();
    }
}

void ChatViewModel::onMessageReceived(const flykylin::core::Message& message) {
    QString peerId = message.fromUserId();

    if (!m_isGroupChat) {
        if (peerId != m_currentPeerId) {
            qDebug() << "[ChatViewModel] Received message from" << peerId
                     << "but current peer is" << m_currentPeerId << ", ignoring";
            return;
        }

        qInfo() << "[ChatViewModel] Message received from" << peerId;

        m_messages.append(message);
        rebuildMessageModel();
        emit messageReceived(message);
        emit messagesUpdated();
    } else {
        if (!m_groupMembers.contains(peerId)) {
            return;
        }

        qInfo() << "[ChatViewModel] Group message received from" << peerId
                << "for group" << m_currentGroupId;

        m_messages.append(message);
        std::sort(m_messages.begin(), m_messages.end(),
                  [](const core::Message& a, const core::Message& b) {
                      return a.timestamp() < b.timestamp();
                  });
        rebuildMessageModel();
        emit messageReceived(message);
        emit messagesUpdated();
    }
}

void ChatViewModel::onMessageSent(const flykylin::core::Message& message) {
    QString peerId = message.toUserId();

    if (!m_isGroupChat) {
        if (peerId != m_currentPeerId) {
            return;
        }

        qInfo() << "[ChatViewModel] Message sent to" << peerId;

        for (auto& msg : m_messages) {
            if (msg.id() == message.id()) {
                msg = message;
                break;
            }
        }

        if (std::none_of(m_messages.begin(), m_messages.end(),
                         [&message](const core::Message& msg) {
                             return msg.id() == message.id();
                         })) {
            m_messages.append(message);
        }

        rebuildMessageModel();
        emit messageSent(message);
        emit messagesUpdated();
    } else {
        if (!m_groupMembers.contains(peerId)) {
            return;
        }

        qInfo() << "[ChatViewModel] Group message sent to" << peerId
                << "for group" << m_currentGroupId;

        for (auto& msg : m_messages) {
            if (msg.id() == message.id()) {
                msg = message;
                break;
            }
        }

        if (std::none_of(m_messages.begin(), m_messages.end(),
                         [&message](const core::Message& msg) {
                             return msg.id() == message.id();
                         })) {
            m_messages.append(message);
        }

        std::sort(m_messages.begin(), m_messages.end(),
                  [](const core::Message& a, const core::Message& b) {
                      return a.timestamp() < b.timestamp();
                  });

        rebuildMessageModel();
        emit messageSent(message);
        emit messagesUpdated();
    }
}

void ChatViewModel::onMessageFailed(const flykylin::core::Message& message, const QString& error) {
    QString peerId = message.toUserId();

    if (!m_isGroupChat) {
        if (peerId != m_currentPeerId) {
            return;
        }

        qWarning() << "[ChatViewModel] Message failed to" << peerId << "error:" << error;

        for (auto& msg : m_messages) {
            if (msg.id() == message.id()) {
                msg = message;
                break;
            }
        }

        rebuildMessageModel();
        emit messageFailed(message, error);
        emit messagesUpdated();
    } else {
        if (!m_groupMembers.contains(peerId)) {
            return;
        }

        qWarning() << "[ChatViewModel] Group message failed to" << peerId
                   << "for group" << m_currentGroupId << "error:" << error;

        for (auto& msg : m_messages) {
            if (msg.id() == message.id()) {
                msg = message;
                break;
            }
        }

        std::sort(m_messages.begin(), m_messages.end(),
                  [](const core::Message& a, const core::Message& b) {
                      return a.timestamp() < b.timestamp();
                  });

        rebuildMessageModel();
        emit messageFailed(message, error);
        emit messagesUpdated();
    }
}

void ChatViewModel::loadMessagesFromService() {
    m_messages = m_messageService->getMessageHistory(m_currentPeerId);
    
    qInfo() << "[ChatViewModel] Loaded" << m_messages.size() << "messages for" << m_currentPeerId;
    
    rebuildMessageModel();
    emit messagesUpdated();
}

void ChatViewModel::resetConversation() {
    qInfo() << "[ChatViewModel] Resetting current conversation";

    m_isGroupChat = false;
    m_currentGroupId.clear();
    m_groupMembers.clear();
    m_currentPeerId.clear();
    m_currentPeerName.clear();
    m_messages.clear();

    rebuildMessageModel();
    emit messagesUpdated();
    emit peerChanged(QString(), QString());
}

} // namespace ui
} // namespace flykylin
