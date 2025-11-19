/**
 * @file LocalEchoService.h
 * @brief Local echo service for testing message flow
 * @author FlyKylin Development Team
 * @date 2024-11-20
 */

#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include "core/models/Message.h"

namespace flykylin {
namespace services {

/**
 * @class LocalEchoService
 * @brief Local echo service to simulate message receiving for testing
 * 
 * This service provides a virtual "Echo Bot" that automatically replies
 * to messages, allowing complete testing of the message flow without
 * needing multiple instances or network connection.
 * 
 * Features:
 * - Virtual peer node "Echo Bot"
 * - Automatic message echo with slight delay
 * - Simulates realistic message receiving
 * - Useful for UI and flow testing
 */
class LocalEchoService : public QObject {
    Q_OBJECT
    
public:
    explicit LocalEchoService(QObject* parent = nullptr);
    ~LocalEchoService() override;
    
    /**
     * @brief Get the Echo Bot user ID
     */
    static QString getEchoBotId() { return "echo_bot_local"; }
    
    /**
     * @brief Get the Echo Bot display name
     */
    static QString getEchoBotName() { return "Echo Bot (Local Test)"; }
    
    /**
     * @brief Send a message to Echo Bot (will trigger auto-reply)
     * @param content Message content
     */
    void sendToEchoBot(const QString& content);
    
    /**
     * @brief Enable or disable echo service
     * @param enabled True to enable, false to disable
     */
    void setEnabled(bool enabled);
    
    /**
     * @brief Check if echo service is enabled
     */
    bool isEnabled() const { return m_enabled; }
    
signals:
    /**
     * @brief Echo message received (simulated incoming message)
     * @param message The echoed message
     */
    void messageReceived(const flykylin::core::Message& message);
    
private slots:
    void onEchoTimerTimeout();
    
private:
    bool m_enabled;
    QTimer* m_echoTimer;
    QString m_pendingContent;
};

} // namespace services
} // namespace flykylin
