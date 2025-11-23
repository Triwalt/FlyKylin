/**
 * @file Message.h
 * @brief 聊天消息数据模型
 * @author FlyKylin Team
 * @date 2024-11-19
 */

#pragma once

#include <QString>
#include <QDateTime>
#include <QtGlobal>

namespace flykylin {
namespace core {

/**
 * @enum MessageStatus
 * @brief 消息状态枚举
 * 
 * 表示消息在发送-接收流程中的各个阶段。
 * 
 * 状态转换流程：
 * @code
 * Sending → Sent (TCP发送成功)
 *        ↘ Failed (TCP发送失败，网络错误等)
 * 
 * Delivered → Read (对方读取消息，未来实现)
 * @endcode
 * 
 * @note 当前实现了Sending、Sent、Failed、Delivered状态
 * @note Read状态预留给未来功能
 * @see Message::status(), Message::setStatus()
 */
enum class MessageStatus {
    Sending,    ///< 发送中（默认状态，消息刚创建）
    Sent,       ///< 已发送（TCP层确认发送成功）
    Delivered,  ///< 已送达（对方确认接收）
    Read,       ///< 已读（对方已读取，未来实现）
    Failed      ///< 发送失败（网络错误、连接断开等）
};

enum class MessageKind {
    Text,
    Image,
    File
};

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
    MessageStatus status() const { return m_status; }
    MessageKind kind() const { return m_kind; }
    QString attachmentLocalPath() const { return m_attachmentLocalPath; }
    QString attachmentName() const { return m_attachmentName; }
    quint64 attachmentSize() const { return m_attachmentSize; }
    QString mimeType() const { return m_mimeType; }

    // Setters
    void setId(const QString& id) { m_id = id; }
    void setFromUserId(const QString& fromUserId) { m_fromUserId = fromUserId; }
    void setToUserId(const QString& toUserId) { m_toUserId = toUserId; }
    void setContent(const QString& content) { m_content = content; }
    void setTimestamp(const QDateTime& timestamp) { m_timestamp = timestamp; }
    void setRead(bool isRead) { m_isRead = isRead; }
    void setStatus(MessageStatus status) { m_status = status; }
    void setKind(MessageKind kind) { m_kind = kind; }
    void setAttachmentLocalPath(const QString& path) { m_attachmentLocalPath = path; }
    void setAttachmentName(const QString& name) { m_attachmentName = name; }
    void setAttachmentSize(quint64 size) { m_attachmentSize = size; }
    void setMimeType(const QString& mimeType) { m_mimeType = mimeType; }

    /**
     * @brief 生成唯一消息ID
     * @return 消息ID (UUID格式)
     */
    static QString generateMessageId();

    // 比较运算符
    bool operator==(const Message& other) const;
    bool operator!=(const Message& other) const;

private:
    QString m_id;                               ///< 消息ID (UUID)
    QString m_fromUserId;                       ///< 发送者用户ID
    QString m_toUserId;                         ///< 接收者用户ID
    QString m_content;                          ///< 消息内容
    QDateTime m_timestamp;                      ///< 发送时间戳
    bool m_isRead{false};                       ///< 是否已读
    MessageStatus m_status{MessageStatus::Sending};  ///< 消息状态
    MessageKind m_kind{MessageKind::Text};
    QString m_attachmentLocalPath;
    QString m_attachmentName;
    quint64 m_attachmentSize{0};
    QString m_mimeType;
};

} // namespace core
} // namespace flykylin

// Register Message as Qt meta-type for use in signals/slots
Q_DECLARE_METATYPE(flykylin::core::Message)
