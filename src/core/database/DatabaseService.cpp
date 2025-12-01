#include "DatabaseService.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

namespace flykylin {
namespace database {

DatabaseService* DatabaseService::instance() {
    static DatabaseService* s_instance = new DatabaseService();
    return s_instance;
}

DatabaseService::DatabaseService(QObject* parent)
    : QObject(parent) {
}

DatabaseService::~DatabaseService() {
    if (m_db.isOpen()) {
        m_db.close();
    }

    const QString connectionName = m_db.connectionName();
    if (!connectionName.isEmpty()) {
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
    }
}

bool DatabaseService::ensureInitialized() const {
    if (m_initialized) {
        return true;
    }

    if (m_initFailed) {
        return false;
    }

    auto self = const_cast<DatabaseService*>(this);
    if (!self->init()) {
        self->m_initFailed = true;
        return false;
    }

    self->m_initialized = true;
    return true;
}

bool DatabaseService::init() {
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDir.isEmpty()) {
        qCritical() << "[DatabaseService] Failed to get AppDataLocation";
        return false;
    }

    QDir dir(baseDir);
    if (!dir.exists("FlyKylin")) {
        if (!dir.mkpath("FlyKylin")) {
            qCritical() << "[DatabaseService] Failed to create directory" << dir.filePath("FlyKylin");
            return false;
        }
    }

    if (!dir.cd("FlyKylin")) {
        qCritical() << "[DatabaseService] Failed to enter directory FlyKylin";
        return false;
    }

    m_dbPath = dir.filePath("chat_history.db");

    if (!QSqlDatabase::contains("FlyKylinChatHistory")) {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "FlyKylinChatHistory");
    } else {
        m_db = QSqlDatabase::database("FlyKylinChatHistory");
    }

    m_db.setDatabaseName(m_dbPath);
    if (!m_db.open()) {
        qCritical() << "[DatabaseService] Failed to open database" << m_dbPath << ":" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS messages ("
            "id TEXT PRIMARY KEY,"
            "local_user_id TEXT NOT NULL,"
            "peer_id TEXT NOT NULL,"
            "from_id TEXT NOT NULL,"
            "to_id TEXT NOT NULL,"
            "content TEXT,"
            "timestamp INTEGER NOT NULL,"
            "status INTEGER NOT NULL,"
            "kind INTEGER NOT NULL,"
            "is_read INTEGER NOT NULL,"
            "attachment_path TEXT,"
            "attachment_name TEXT,"
            "attachment_size INTEGER,"
            "mime_type TEXT"
            ")")) {
        qCritical() << "[DatabaseService] Failed to create messages table:" << query.lastError().text();
        return false;
    }

    if (!query.exec(
            "CREATE INDEX IF NOT EXISTS idx_messages_peer_ts "
            "ON messages(local_user_id, peer_id, timestamp)")) {
        qWarning() << "[DatabaseService] Failed to create idx_messages_peer_ts:" << query.lastError().text();
    }

    if (!query.exec(
            "CREATE INDEX IF NOT EXISTS idx_messages_ts "
            "ON messages(local_user_id, timestamp)")) {
        qWarning() << "[DatabaseService] Failed to create idx_messages_ts:" << query.lastError().text();
    }

    // 会话表：记录每个本地用户与哪些peer有过对话，以及最近一条消息时间
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS sessions ("
            "local_user_id TEXT NOT NULL,"
            "peer_id TEXT NOT NULL,"
            "last_timestamp INTEGER NOT NULL,"
            "PRIMARY KEY(local_user_id, peer_id)"
            ")")) {
        qCritical() << "[DatabaseService] Failed to create sessions table:" << query.lastError().text();
        return false;
    }

    if (!query.exec(
            "CREATE INDEX IF NOT EXISTS idx_sessions_ts "
            "ON sessions(local_user_id, last_timestamp)")) {
        qWarning() << "[DatabaseService] Failed to create idx_sessions_ts:" << query.lastError().text();
    }

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS peers ("
            "user_id TEXT PRIMARY KEY,"
            "user_name TEXT,"
            "host_name TEXT,"
            "ip_address TEXT,"
            "tcp_port INTEGER,"
            "last_seen INTEGER"
            ")")) {
        qCritical() << "[DatabaseService] Failed to create peers table:" << query.lastError().text();
        return false;
    }

    qInfo() << "[DatabaseService] Initialized chat history database at" << m_dbPath;

    return true;
}

QList<core::Message> DatabaseService::loadMessages(const QString& localUserId, const QString& peerId) const {
    QList<core::Message> result;

    if (!ensureInitialized()) {
        return result;
    }

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT id, from_id, to_id, content, timestamp, status, kind, is_read, "
        "attachment_path, attachment_name, attachment_size, mime_type "
        "FROM messages "
        "WHERE local_user_id = :local_user_id AND peer_id = :peer_id "
        "ORDER BY timestamp ASC, rowid ASC");
    query.bindValue(":local_user_id", localUserId);
    query.bindValue(":peer_id", peerId);

    if (!query.exec()) {
        qWarning() << "[DatabaseService] Failed to load messages for" << localUserId << peerId
                   << ":" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        core::Message message;
        message.setId(query.value(0).toString());
        message.setFromUserId(query.value(1).toString());
        message.setToUserId(query.value(2).toString());
        message.setContent(query.value(3).toString());

        const qint64 ts = query.value(4).toLongLong();
        message.setTimestamp(QDateTime::fromMSecsSinceEpoch(ts));

        const int statusValue = query.value(5).toInt();
        if (statusValue >= static_cast<int>(core::MessageStatus::Sending) &&
            statusValue <= static_cast<int>(core::MessageStatus::Failed)) {
            message.setStatus(static_cast<core::MessageStatus>(statusValue));
        }

        const int kindValue = query.value(6).toInt();
        if (kindValue >= static_cast<int>(core::MessageKind::Text) &&
            kindValue <= static_cast<int>(core::MessageKind::File)) {
            message.setKind(static_cast<core::MessageKind>(kindValue));
        }

        const bool isRead = query.value(7).toInt() != 0;
        message.setRead(isRead);

        message.setAttachmentLocalPath(query.value(8).toString());
        message.setAttachmentName(query.value(9).toString());
        message.setAttachmentSize(query.value(10).toULongLong());
        message.setMimeType(query.value(11).toString());

        result.append(message);
    }

    qInfo() << "[DatabaseService] Loaded" << result.size() << "messages for" << localUserId << peerId;

    return result;
}

QList<core::Message> DatabaseService::searchMessagesByKeyword(const QString& localUserId,
                                                              const QString& keyword,
                                                              const QString& peerId,
                                                              int limit) const {
    QList<core::Message> result;

    if (!ensureInitialized()) {
        return result;
    }

    const QString trimmed = keyword.trimmed();
    if (trimmed.isEmpty()) {
        return result;
    }

    QSqlQuery query(m_db);

    QString sql =
        "SELECT id, from_id, to_id, content, timestamp, status, kind, is_read, "
        "attachment_path, attachment_name, attachment_size, mime_type "
        "FROM messages "
        "WHERE local_user_id = :local_user_id AND content LIKE :pattern ";

    if (!peerId.isEmpty()) {
        sql += "AND peer_id = :peer_id ";
    }

    sql += "ORDER BY timestamp DESC, rowid DESC LIMIT :limit";

    query.prepare(sql);
    query.bindValue(":local_user_id", localUserId);
    query.bindValue(":pattern", QString("%%1%").arg(trimmed));
    if (!peerId.isEmpty()) {
        query.bindValue(":peer_id", peerId);
    }
    const int effectiveLimit = (limit > 0) ? limit : 200;
    query.bindValue(":limit", effectiveLimit);

    if (!query.exec()) {
        qWarning() << "[DatabaseService] Failed to search messages for" << localUserId
                   << "keyword=" << trimmed << "peer=" << peerId
                   << ":" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        core::Message message;
        message.setId(query.value(0).toString());
        message.setFromUserId(query.value(1).toString());
        message.setToUserId(query.value(2).toString());
        message.setContent(query.value(3).toString());

        const qint64 ts = query.value(4).toLongLong();
        message.setTimestamp(QDateTime::fromMSecsSinceEpoch(ts));

        const int statusValue = query.value(5).toInt();
        if (statusValue >= static_cast<int>(core::MessageStatus::Sending) &&
            statusValue <= static_cast<int>(core::MessageStatus::Failed)) {
            message.setStatus(static_cast<core::MessageStatus>(statusValue));
        }

        const int kindValue = query.value(6).toInt();
        if (kindValue >= static_cast<int>(core::MessageKind::Text) &&
            kindValue <= static_cast<int>(core::MessageKind::File)) {
            message.setKind(static_cast<core::MessageKind>(kindValue));
        }

        const bool isRead = query.value(7).toInt() != 0;
        message.setRead(isRead);

        message.setAttachmentLocalPath(query.value(8).toString());
        message.setAttachmentName(query.value(9).toString());
        message.setAttachmentSize(query.value(10).toULongLong());
        message.setMimeType(query.value(11).toString());

        result.append(message);
    }

    qInfo() << "[DatabaseService] Keyword search returned" << result.size() << "messages for"
            << localUserId << "peer=" << peerId << "keyword=" << trimmed;

    return result;
}

void DatabaseService::appendMessage(const core::Message& message, const QString& localUserId) {
    if (!ensureInitialized()) {
        return;
    }

    const QString peerId = (message.fromUserId() == localUserId)
                               ? message.toUserId()
                               : message.fromUserId();

    QSqlQuery query(m_db);
    query.prepare(
        "INSERT OR REPLACE INTO messages ("
        "id, local_user_id, peer_id, from_id, to_id, content, timestamp, status, kind, is_read, "
        "attachment_path, attachment_name, attachment_size, mime_type"
        ") VALUES ("
        ":id, :local_user_id, :peer_id, :from_id, :to_id, :content, :timestamp, :status, :kind, :is_read, "
        ":attachment_path, :attachment_name, :attachment_size, :mime_type"
        ")");

    query.bindValue(":id", message.id());
    query.bindValue(":local_user_id", localUserId);
    query.bindValue(":peer_id", peerId);
    query.bindValue(":from_id", message.fromUserId());
    query.bindValue(":to_id", message.toUserId());
    query.bindValue(":content", message.content());
    query.bindValue(":timestamp", message.timestamp().toMSecsSinceEpoch());
    query.bindValue(":status", static_cast<int>(message.status()));
    query.bindValue(":kind", static_cast<int>(message.kind()));
    query.bindValue(":is_read", message.isRead() ? 1 : 0);
    query.bindValue(":attachment_path", message.attachmentLocalPath());
    query.bindValue(":attachment_name", message.attachmentName());
    query.bindValue(":attachment_size", static_cast<qint64>(message.attachmentSize()));
    query.bindValue(":mime_type", message.mimeType());

    if (!query.exec()) {
        qWarning() << "[DatabaseService] Failed to append message" << message.id()
                   << ":" << query.lastError().text();
    }
}

QList<QPair<QString, qint64>> DatabaseService::loadSessions(const QString& localUserId) const {
    QList<QPair<QString, qint64>> result;

    if (!ensureInitialized()) {
        return result;
    }

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT peer_id, last_timestamp "
        "FROM sessions "
        "WHERE local_user_id = :local_user_id "
        "ORDER BY last_timestamp DESC");
    query.bindValue(":local_user_id", localUserId);

    if (!query.exec()) {
        qWarning() << "[DatabaseService] Failed to load sessions for" << localUserId
                   << ":" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        const QString peerId = query.value(0).toString();
        const qint64 ts = query.value(1).toLongLong();
        result.append(qMakePair(peerId, ts));
    }

    qInfo() << "[DatabaseService] Loaded" << result.size() << "sessions for" << localUserId;

    return result;
}

void DatabaseService::touchSession(const QString& localUserId,
                                   const QString& peerId,
                                   qint64 lastTimestamp) {
    if (!ensureInitialized()) {
        return;
    }

    QSqlQuery query(m_db);
    query.prepare(
        "INSERT OR REPLACE INTO sessions (local_user_id, peer_id, last_timestamp) "
        "VALUES (:local_user_id, :peer_id, :last_timestamp)");
    query.bindValue(":local_user_id", localUserId);
    query.bindValue(":peer_id", peerId);
    query.bindValue(":last_timestamp", lastTimestamp);

    if (!query.exec()) {
        qWarning() << "[DatabaseService] Failed to touch session for" << localUserId << peerId
                   << ":" << query.lastError().text();
    }
}

void DatabaseService::clearHistory(const QString& localUserId, const QString& peerId) {
    if (!ensureInitialized()) {
        return;
    }

    QSqlQuery query(m_db);
    query.prepare(
        "DELETE FROM messages "
        "WHERE local_user_id = :local_user_id AND peer_id = :peer_id");
    query.bindValue(":local_user_id", localUserId);
    query.bindValue(":peer_id", peerId);

    if (!query.exec()) {
        qWarning() << "[DatabaseService] Failed to clear history for" << localUserId << peerId
                   << ":" << query.lastError().text();
    }

    // Also remove the session entry so that the conversation disappears from the session list
    QSqlQuery sessionsQuery(m_db);
    sessionsQuery.prepare(
        "DELETE FROM sessions "
        "WHERE local_user_id = :local_user_id AND peer_id = :peer_id");
    sessionsQuery.bindValue(":local_user_id", localUserId);
    sessionsQuery.bindValue(":peer_id", peerId);

    if (!sessionsQuery.exec()) {
        qWarning() << "[DatabaseService] Failed to clear session for" << localUserId << peerId
                   << ":" << sessionsQuery.lastError().text();
    }
}

void DatabaseService::upsertPeer(const PeerInfo& info) {
    if (!ensureInitialized()) {
        return;
    }

    QSqlQuery query(m_db);
    query.prepare(
        "INSERT OR REPLACE INTO peers (user_id, user_name, host_name, ip_address, tcp_port, last_seen) "
        "VALUES (:user_id, :user_name, :host_name, :ip_address, :tcp_port, :last_seen)");

    query.bindValue(":user_id", info.userId);
    query.bindValue(":user_name", info.userName);
    query.bindValue(":host_name", info.hostName);
    query.bindValue(":ip_address", info.ipAddress);
    query.bindValue(":tcp_port", static_cast<int>(info.tcpPort));

    qint64 ts = info.lastSeen;
    if (ts <= 0) {
        ts = QDateTime::currentMSecsSinceEpoch();
    }
    query.bindValue(":last_seen", ts);

    if (!query.exec()) {
        qWarning() << "[DatabaseService] Failed to upsert peer" << info.userId
                   << ":" << query.lastError().text();
    }
}

bool DatabaseService::loadPeer(const QString& userId, PeerInfo& outInfo) const {
    if (!ensureInitialized()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT user_id, user_name, host_name, ip_address, tcp_port, last_seen "
        "FROM peers WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qWarning() << "[DatabaseService] Failed to load peer" << userId
                   << ":" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        return false;
    }

    outInfo.userId = query.value(0).toString();
    outInfo.userName = query.value(1).toString();
    outInfo.hostName = query.value(2).toString();
    outInfo.ipAddress = query.value(3).toString();
    outInfo.tcpPort = static_cast<quint16>(query.value(4).toInt());
    outInfo.lastSeen = query.value(5).toLongLong();

    return true;
}

} // namespace database
} // namespace flykylin
