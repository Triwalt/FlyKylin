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
#include "../../core/database/DatabaseService.h"
#include "../../core/ai/NSFWDetector.h"
#include <QDateTime>
#include <QDebug>
#include <QUrl>
#include <QGuiApplication>
#include <QScreen>
#include <QStandardPaths>
#include <QDir>
#include <QPixmap>
#include <QImage>
#include <QFile>
#include <QSettings>

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
    roles[Qt::UserRole + 8] = "messageId";
    roles[Qt::UserRole + 9] = "nsfwPassed";
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
    QString localUserName = core::UserProfile::instance().userName();
    if (localUserName.isEmpty()) {
        localUserName = localUserId;
    }
    auto* db = flykylin::database::DatabaseService::instance();
    flykylin::database::DatabaseService::PeerInfo peerInfo;
    
    for (const auto& msg : m_messages) {
        auto* item = new QStandardItem();
        item->setData(msg.content(), Qt::UserRole);
        const bool isMine = (msg.fromUserId() == localUserId);
        item->setData(isMine, Qt::UserRole + 1);
        item->setData(msg.timestamp().toString("HH:mm:ss"), Qt::UserRole + 2);
        item->setData(QVariant::fromValue(msg.status()), Qt::UserRole + 3);
        QString senderName;
        if (isMine) {
            senderName = localUserName;
        } else if (!m_isGroupChat && !m_currentPeerName.isEmpty()) {
            senderName = m_currentPeerName;
        } else if (db && db->loadPeer(msg.fromUserId(), peerInfo) && !peerInfo.userName.isEmpty()) {
            senderName = peerInfo.userName;
        } else {
            senderName = msg.fromUserId();
        }
        item->setData(senderName, Qt::UserRole + 4);

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
        item->setData(msg.id(), Qt::UserRole + 8);
        item->setData(msg.nsfwPassed(), Qt::UserRole + 9);
        m_messageModel->appendRow(item);
    }
}

ChatViewModel::~ChatViewModel() {
    qInfo() << "[ChatViewModel] Destroyed";
}

void ChatViewModel::setCurrentPeer(const QString& peerId, const QString& peerName) {
    const bool samePeer = (m_currentPeerId == peerId);

    qInfo() << "[ChatViewModel] Switching to peer" << peerId << "(" << peerName << ")";

    m_isGroupChat = false;
    m_currentGroupId.clear();
    m_groupMembers.clear();
    m_currentPeerId = peerId;
    m_currentPeerName = peerName;

    if (!samePeer) {
        loadMessagesFromService();
    } else {
        rebuildMessageModel();
        emit messagesUpdated();
    }

    emit peerChanged(peerId, peerName);
}

void ChatViewModel::setCurrentGroup(const QString& groupId,
                                    const QString& groupName,
                                    const QStringList& memberIds) {
    if (groupId.isEmpty()) {
        qWarning() << "[ChatViewModel] setCurrentGroup: invalid group id";
        return;
    }

    QStringList effectiveMembers = memberIds;

    if (effectiveMembers.isEmpty()) {
        if (m_isGroupChat && groupId == m_currentGroupId && !m_groupMembers.isEmpty()) {
            qWarning() << "[ChatViewModel] setCurrentGroup: empty memberIds for existing group"
                       << groupId << ", reusing previous members" << m_groupMembers;
            effectiveMembers = m_groupMembers;
        } else {
            qWarning() << "[ChatViewModel] setCurrentGroup: group" << groupId
                       << "has no members yet, opening empty group view";
        }
    }

    qInfo() << "[ChatViewModel] Switching to group" << groupId << "(" << groupName
            << ") members=" << effectiveMembers;

    m_isGroupChat = true;
    m_currentGroupId = groupId;
    m_groupMembers = effectiveMembers;
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

void ChatViewModel::sendScreenshot() {
    if (!m_isGroupChat && m_currentPeerId.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send screenshot: no peer selected";
        return;
    }

    if (m_isGroupChat && m_groupMembers.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send screenshot: group has no members";
        return;
    }

    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) {
        qWarning() << "[ChatViewModel] Cannot send screenshot: no primary screen";
        return;
    }

    QPixmap pixmap = screen->grabWindow(0);
    if (pixmap.isNull()) {
        qWarning() << "[ChatViewModel] Cannot send screenshot: grabWindow returned null";
        return;
    }

    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (baseDir.isEmpty()) {
        baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }

    if (baseDir.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send screenshot: no writable location";
        return;
    }

    QDir dir(baseDir);
    if (!dir.exists("FlyKylin")) {
        if (!dir.mkpath("FlyKylin")) {
            qWarning() << "[ChatViewModel] Cannot send screenshot: failed to create directory"
                       << dir.absolutePath();
            return;
        }
        dir.cd("FlyKylin");
    } else {
        dir.cd("FlyKylin");
    }

    const QString fileName =
        QStringLiteral("screenshot_%1.png")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    const QString filePath = dir.filePath(fileName);

    if (!pixmap.save(filePath, "PNG")) {
        qWarning() << "[ChatViewModel] Cannot send screenshot: failed to save to" << filePath;
        return;
    }

    sendImage(filePath);
}

QString ChatViewModel::captureScreenForSelection() {
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) {
        qWarning() << "[ChatViewModel] Cannot capture screen: no primary screen";
        return QString();
    }

    QPixmap pixmap = screen->grabWindow(0);
    if (pixmap.isNull()) {
        qWarning() << "[ChatViewModel] Cannot capture screen: grabWindow returned null";
        return QString();
    }

    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (baseDir.isEmpty()) {
        baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }

    if (baseDir.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot capture screen: no writable location";
        return QString();
    }

    QDir dir(baseDir);
    if (!dir.exists("FlyKylin")) {
        if (!dir.mkpath("FlyKylin")) {
            qWarning() << "[ChatViewModel] Cannot capture screen: failed to create directory"
                       << dir.absolutePath();
            return QString();
        }
    }

    if (!dir.cd("FlyKylin")) {
        qWarning() << "[ChatViewModel] Cannot capture screen: failed to enter directory"
                   << dir.absolutePath();
        return QString();
    }

    const QString fileName =
        QStringLiteral("screenshot_full_%1.png")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    const QString filePath = dir.filePath(fileName);

    if (!pixmap.save(filePath, "PNG")) {
        qWarning() << "[ChatViewModel] Cannot capture screen: failed to save to" << filePath;
        return QString();
    }

    qInfo() << "[ChatViewModel] Captured full screen to" << filePath;
    return filePath;
}

void ChatViewModel::sendCroppedScreenshot(const QString& fullPath,
                                          int x,
                                          int y,
                                          int width,
                                          int height) {
    if (!m_isGroupChat && m_currentPeerId.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send cropped screenshot: no peer selected";
        return;
    }

    if (m_isGroupChat && m_groupMembers.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send cropped screenshot: group has no members";
        return;
    }

    if (fullPath.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send cropped screenshot: empty path";
        return;
    }

    if (width <= 0 || height <= 0) {
        qWarning() << "[ChatViewModel] Cannot send cropped screenshot: invalid size" << width
                   << height;
        return;
    }

    QImage image(fullPath);
    if (image.isNull()) {
        qWarning() << "[ChatViewModel] Cannot send cropped screenshot: failed to load"
                   << fullPath;
        return;
    }

    QRect cropRect(x, y, width, height);
    cropRect = cropRect.intersected(image.rect());
    if (!cropRect.isValid() || cropRect.width() <= 0 || cropRect.height() <= 0) {
        qWarning() << "[ChatViewModel] Cannot send cropped screenshot: crop rect invalid"
                   << cropRect;
        return;
    }

    QImage cropped = image.copy(cropRect);

    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (baseDir.isEmpty()) {
        baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }

    if (baseDir.isEmpty()) {
        qWarning() << "[ChatViewModel] Cannot send cropped screenshot: no writable location";
        return;
    }

    QDir dir(baseDir);
    if (!dir.exists("FlyKylin")) {
        if (!dir.mkpath("FlyKylin")) {
            qWarning() << "[ChatViewModel] Cannot send cropped screenshot: failed to create"
                       << dir.absolutePath();
            return;
        }
    }

    if (!dir.cd("FlyKylin")) {
        qWarning() << "[ChatViewModel] Cannot send cropped screenshot: failed to enter directory"
                   << dir.absolutePath();
        return;
    }

    const QString fileName =
        QStringLiteral("screenshot_%1.png")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    const QString filePath = dir.filePath(fileName);

    if (!cropped.save(filePath, "PNG")) {
        qWarning() << "[ChatViewModel] Cannot send cropped screenshot: failed to save to"
                   << filePath;
        return;
    }

    // Cleanup temporary full-screen capture; ignore errors
    QFile::remove(fullPath);

    qInfo() << "[ChatViewModel] Sending cropped screenshot from" << filePath;
    sendImage(filePath);
}

void ChatViewModel::deleteConversation(const QString& peerId) {
    if (peerId.isEmpty()) {
        return;
    }

    qInfo() << "[ChatViewModel] Deleting conversation for" << peerId;

    m_messageService->clearHistory(peerId);

    if (!m_isGroupChat && m_currentPeerId == peerId) {
        resetConversation();
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

int ChatViewModel::findMessageRow(const QString& messageId) const {
    if (messageId.isEmpty()) {
        return -1;
    }

    for (int i = 0; i < m_messages.size(); ++i) {
        if (m_messages.at(i).id() == messageId) {
            return i;
        }
    }

    return -1;
}

bool ChatViewModel::isImageNsfw(const QString& filePath) const
{
    QString normalizedPath = filePath;
    if (filePath.startsWith("file:")) {
        const QUrl url(filePath);
        const QString local = url.toLocalFile();
        if (!local.isEmpty()) {
            normalizedPath = local;
        }
    }

    ai::NSFWDetector* detector = ai::NSFWDetector::instance();
    if (!detector || !detector->isAvailable()) {
        qInfo() << "[ChatViewModel] NSFWDetector not available, skipping check for" << normalizedPath;
        return false;
    }

    const auto prob = detector->predictNsfwProbability(normalizedPath);
    if (!prob.has_value()) {
        qWarning() << "[ChatViewModel] NSFW detection failed for emoji" << normalizedPath;
        return false;
    }

    QSettings settings("FlyKylin", "FlyKylin");
    double threshold = settings.value("nsfw/threshold", 0.8).toDouble();
    if (threshold < 0.0) {
        threshold = 0.0;
    } else if (threshold > 1.0) {
        threshold = 1.0;
    }

    qInfo() << "[ChatViewModel] NSFW probability for emoji" << normalizedPath << "=" << *prob
            << "threshold=" << threshold;
    return *prob >= static_cast<float>(threshold);
}

} // namespace ui
} // namespace flykylin
