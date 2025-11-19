/**
 * @file MessageQueue.cpp
 * @brief Priority message queue implementation
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#include "MessageQueue.h"
#include <QDebug>

namespace flykylin {
namespace communication {

MessageQueue::MessageQueue(QObject* parent)
    : QObject(parent)
    , m_nextMessageId(1)
{
    qDebug() << "[MessageQueue] Created";
}

void MessageQueue::enqueue(const QByteArray& data, Priority priority) {
    // Check queue size limit
    if (size() >= kMaxQueueSize) {
        qCritical() << "[MessageQueue] Queue full, dropping message (priority=" 
                    << static_cast<int>(priority) << ")";
        return;
    }
    
    QueuedMessage msg;
    msg.priority = priority;
    msg.messageId = generateMessageId();
    msg.data = data;
    msg.enqueueTime = QDateTime::currentDateTime();
    msg.retryCount = 0;
    
    m_priorityQueue.insert(priority, msg);
    
    qDebug() << "[MessageQueue] Enqueued message id=" << msg.messageId 
             << "priority=" << static_cast<int>(priority)
             << "size=" << data.size()
             << "queue_size=" << size();
    
    emit messageEnqueued();
}

MessageQueue::QueuedMessage MessageQueue::dequeue() {
    // Try retry queue first
    if (!m_retryQueue.isEmpty()) {
        QueuedMessage msg = m_retryQueue.dequeue();
        qDebug() << "[MessageQueue] Dequeued from retry queue id=" << msg.messageId;
        return msg;
    }
    
    // Then try priority queue
    if (m_priorityQueue.isEmpty()) {
        qDebug() << "[MessageQueue] Queue empty, returning null message";
        return QueuedMessage();  // Empty message
    }
    
    // Get highest priority message (lowest priority enum value)
    auto it = m_priorityQueue.begin();
    QueuedMessage msg = it.value();
    m_priorityQueue.erase(it);
    
    qDebug() << "[MessageQueue] Dequeued message id=" << msg.messageId
             << "priority=" << static_cast<int>(msg.priority)
             << "queue_size=" << size();
    
    return msg;
}

bool MessageQueue::requeueForRetry(const QueuedMessage& msg) {
    if (msg.retryCount >= kMaxRetryCount) {
        qWarning() << "[MessageQueue] Max retries reached for message id=" << msg.messageId 
                   << "retry_count=" << msg.retryCount;
        return false;
    }
    
    QueuedMessage retryMsg = msg;
    retryMsg.retryCount++;
    retryMsg.enqueueTime = QDateTime::currentDateTime();
    
    m_retryQueue.enqueue(retryMsg);
    
    qInfo() << "[MessageQueue] Requeued for retry id=" << retryMsg.messageId
            << "retry_count=" << retryMsg.retryCount;
    
    emit messageEnqueued();
    return true;
}

int MessageQueue::size() const {
    return m_priorityQueue.size() + m_retryQueue.size();
}

bool MessageQueue::isEmpty() const {
    return m_priorityQueue.isEmpty() && m_retryQueue.isEmpty();
}

void MessageQueue::clear() {
    int count = size();
    m_priorityQueue.clear();
    m_retryQueue.clear();
    qInfo() << "[MessageQueue] Cleared" << count << "messages";
}

quint64 MessageQueue::generateMessageId() {
    return m_nextMessageId++;
}

} // namespace communication
} // namespace flykylin
