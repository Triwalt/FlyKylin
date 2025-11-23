#pragma once

#include <QObject>
#include <QList>
#include <QString>
#include <QSqlDatabase>

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
