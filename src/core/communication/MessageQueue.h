/**
 * @file MessageQueue.h
 * @brief Priority message queue for TCP connections
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#pragma once

#include <QObject>
#include <QMultiMap>
#include <QQueue>
#include <QDateTime>
#include <QByteArray>

namespace flykylin {
namespace communication {

/**
 * @brief Priority message queue with retry support
 * 
 * Features:
 * - 4 priority levels: Critical > High > Normal > Low
 * - Automatic retry for failed messages (max 3 times)
 * - Message deduplication based on ID
 * - FIFO within same priority level
 */
class MessageQueue : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Message priority levels
     */
    enum class Priority {
        Critical = 0,  ///< ACK, PONG - highest priority
        High = 1,      ///< User text messages
        Normal = 2,    ///< Heartbeat, status updates
        Low = 3        ///< File transfer, background tasks
    };
    
    /**
     * @brief Queued message structure
     */
    struct QueuedMessage {
        Priority priority;        ///< Message priority
        quint64 messageId;        ///< Unique message ID
        QByteArray data;          ///< Message data
        QDateTime enqueueTime;    ///< When message was queued
        int retryCount;           ///< Number of retry attempts
        
        bool operator<(const QueuedMessage& other) const {
            if (priority != other.priority) {
                return priority < other.priority;  // Lower value = higher priority
            }
            return enqueueTime < other.enqueueTime;  // FIFO within same priority
        }
    };
    
    explicit MessageQueue(QObject* parent = nullptr);
    
    /**
     * @brief Add message to queue
     * @param data Message data
     * @param priority Message priority
     */
    void enqueue(const QByteArray& data, Priority priority);
    
    /**
     * @brief Remove and return highest priority message
     * @return Message data or empty QueuedMessage if queue is empty
     */
    QueuedMessage dequeue();
    
    /**
     * @brief Re-queue failed message for retry
     * @param msg Message to retry
     * @return true if message was re-queued, false if max retries reached
     */
    bool requeueForRetry(const QueuedMessage& msg);
    
    /**
     * @brief Get queue size
     */
    int size() const;
    
    /**
     * @brief Check if queue is empty
     */
    bool isEmpty() const;
    
    /**
     * @brief Clear all messages
     */
    void clear();
    
signals:
    /**
     * @brief Emitted when a message is enqueued
     */
    void messageEnqueued();
    
private:
    quint64 generateMessageId();
    
    QMultiMap<Priority, QueuedMessage> m_priorityQueue;  ///< Priority queue
    QQueue<QueuedMessage> m_retryQueue;                  ///< Retry queue
    quint64 m_nextMessageId;                             ///< Next message ID
    
    static constexpr int kMaxQueueSize = 1000;    ///< Max total queue size
    static constexpr int kMaxRetryCount = 3;      ///< Max retry attempts
};

} // namespace communication
} // namespace flykylin
