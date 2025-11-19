/**
 * @file Message.h
 * @brief 聊天消息数据模型
 * @author FlyKylin Team
 * @date 2024-11-19
 */

#pragma once

#include <QString>
#include <QDateTime>

namespace flykylin {
namespace core {

/**
 * @class Message
 * @brief 表示一条文本消息
 * 
 * 用于1v1文本聊天的数据模型，支持序列化和持久化
 */
class Message {
public:
    /**
     * @brief 默认构造函数
     */
    Message() = default;

    /**
     * @brief 构造函数
     * @param id 消息ID
     * @param fromUserId 发送者用户ID
     * @param toUserId 接收者用户ID
     * @param content 消息内容
     * @param timestamp 发送时间戳
     */
    Message(const QString& id,
            const QString& fromUserId,
            const QString& toUserId,
            const QString& content,
            const QDateTime& timestamp);

    // Getters
    QString id() const { return m_id; }
    QString fromUserId() const { return m_fromUserId; }
    QString toUserId() const { return m_toUserId; }
    QString content() const { return m_content; }
    QDateTime timestamp() const { return m_timestamp; }
    bool isRead() const { return m_isRead; }

    // Setters
    void setId(const QString& id) { m_id = id; }
    void setFromUserId(const QString& fromUserId) { m_fromUserId = fromUserId; }
    void setToUserId(const QString& toUserId) { m_toUserId = toUserId; }
    void setContent(const QString& content) { m_content = content; }
    void setTimestamp(const QDateTime& timestamp) { m_timestamp = timestamp; }
    void setRead(bool isRead) { m_isRead = isRead; }

    /**
     * @brief 生成唯一消息ID
     * @return 消息ID (UUID格式)
     */
    static QString generateMessageId();

    // 比较运算符
    bool operator==(const Message& other) const;
    bool operator!=(const Message& other) const;

private:
    QString m_id;               ///< 消息ID (UUID)
    QString m_fromUserId;       ///< 发送者用户ID
    QString m_toUserId;         ///< 接收者用户ID
    QString m_content;          ///< 消息内容
    QDateTime m_timestamp;      ///< 发送时间戳
    bool m_isRead{false};       ///< 是否已读
};

} // namespace core
} // namespace flykylin

// Register Message as Qt meta-type for use in signals/slots
Q_DECLARE_METATYPE(flykylin::core::Message)
