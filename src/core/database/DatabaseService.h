#pragma once

#include <QObject>
#include <QList>
#include <QString>
#include <QSqlDatabase>
#include <QPair>

#include "core/models/Message.h"

namespace flykylin {
namespace database {

class DatabaseService : public QObject {
    Q_OBJECT
public:
    static DatabaseService* instance();

    // 按远端用户维度持久化节点身份信息（用户名、主机名、IP、端口等）
    struct PeerInfo {
        QString userId;
        QString userName;
        QString hostName;
        QString ipAddress;
        quint16 tcpPort{0};
        qint64 lastSeen{0};
    };

    QList<core::Message> loadMessages(const QString& localUserId, const QString& peerId) const;
    void appendMessage(const core::Message& message, const QString& localUserId);
    void clearHistory(const QString& localUserId, const QString& peerId);

    // Simple keyword-based search over message content for a given local user.
    // If peerId is empty, search across all peers. Results are ordered by
    // timestamp DESC and limited by the provided limit (default 200).
    QList<core::Message> searchMessagesByKeyword(const QString& localUserId,
                                                 const QString& keyword,
                                                 const QString& peerId = QString(),
                                                 int limit = 200) const;

    // 会话列表：按本地用户维度记录与哪些peer有过对话，以及最近时间戳
    QList<QPair<QString, qint64>> loadSessions(const QString& localUserId) const;
    void touchSession(const QString& localUserId, const QString& peerId, qint64 lastTimestamp);

    void upsertPeer(const PeerInfo& info);
    bool loadPeer(const QString& userId, PeerInfo& outInfo) const;

private:
    explicit DatabaseService(QObject* parent = nullptr);
    ~DatabaseService() override;

    bool init();
    bool ensureInitialized() const;

    bool m_initialized{false};
    bool m_initFailed{false};
    QSqlDatabase m_db;
    QString m_dbPath;
};

} // namespace database
} // namespace flykylin
