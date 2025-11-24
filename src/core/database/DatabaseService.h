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

    QList<core::Message> loadMessages(const QString& localUserId, const QString& peerId) const;
    void appendMessage(const core::Message& message, const QString& localUserId);
    void clearHistory(const QString& localUserId, const QString& peerId);

    // 会话列表：按本地用户维度记录与哪些peer有过对话，以及最近时间戳
    QList<QPair<QString, qint64>> loadSessions(const QString& localUserId) const;
    void touchSession(const QString& localUserId, const QString& peerId, qint64 lastTimestamp);

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
