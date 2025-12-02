/**
 * @file Message.cpp
 * @brief 聊天消息数据模型实现
 */

#include "Message.h"

namespace flykylin {
namespace core {

Message::Message(const QString& id,
                 const QString& fromUserId,
                 const QString& toUserId,
                 const QString& content,
                 const QDateTime& timestamp)
    : m_id(id)
    , m_fromUserId(fromUserId)
    , m_toUserId(toUserId)
    , m_content(content)
    , m_timestamp(timestamp)
    , m_isRead(false)
    , m_status(MessageStatus::Sending)
{
}

QString Message::generateMessageId()
{
    // 使用当前时间戳和进程内递增计数生成唯一ID，避免依赖 QUuid/QRandomGenerator，
    // 以兼容当前 Linux Qt5 + libstdc++ 环境。
    static quint64 counter = 0;

    const qint64 timestamp = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    const quint64 value = ++counter;

    return QString::number(timestamp, 16) + QLatin1Char('-') + QString::number(value, 16);
}

bool Message::operator==(const Message& other) const
{
    return m_id == other.m_id &&
           m_fromUserId == other.m_fromUserId &&
           m_toUserId == other.m_toUserId &&
           m_content == other.m_content &&
           m_timestamp == other.m_timestamp &&
           m_isRead == other.m_isRead &&
           m_status == other.m_status &&
           m_kind == other.m_kind &&
           m_attachmentLocalPath == other.m_attachmentLocalPath &&
           m_attachmentName == other.m_attachmentName &&
           m_attachmentSize == other.m_attachmentSize &&
           m_mimeType == other.m_mimeType &&
           m_nsfwChecked == other.m_nsfwChecked &&
           m_nsfwPassed == other.m_nsfwPassed;
}

bool Message::operator!=(const Message& other) const
{
    return !(*this == other);
}

} // namespace core
} // namespace flykylin
