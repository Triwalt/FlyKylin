/**
 * @file LocalEchoService.cpp
 * @brief Local echo service implementation
 * @author FlyKylin Development Team
 * @date 2024-11-20
 */

#include "LocalEchoService.h"
#include "../models/Message.h"
#include "../config/UserProfile.h"
#include <QDebug>
#include <QDateTime>
#include <QRandomGenerator>

namespace flykylin {
namespace services {

LocalEchoService::LocalEchoService(QObject* parent)
    : QObject(parent)
    , m_enabled(true)
    , m_echoTimer(new QTimer(this))
{
    m_echoTimer->setSingleShot(true);
    connect(m_echoTimer, &QTimer::timeout,
            this, &LocalEchoService::onEchoTimerTimeout);
    
    qInfo() << "[LocalEchoService] Created - Echo Bot ready for testing";
}

LocalEchoService::~LocalEchoService()
{
    qInfo() << "[LocalEchoService] Destroyed";
}

void LocalEchoService::sendToEchoBot(const QString& content)
{
    if (!m_enabled) {
        qDebug() << "[LocalEchoService] Echo service disabled, ignoring message";
        return;
    }
    
    qInfo() << "[LocalEchoService] Received message for Echo Bot:" << content.left(50);
    
    // Store the content and start timer (simulate network delay)
    m_pendingContent = content;
    
    // Random delay between 500ms-1500ms to simulate network
    int delay = QRandomGenerator::global()->bounded(500, 1500);
    m_echoTimer->start(delay);
    
    qDebug() << "[LocalEchoService] Will echo back in" << delay << "ms";
}

void LocalEchoService::setEnabled(bool enabled)
{
    m_enabled = enabled;
    qInfo() << "[LocalEchoService] Echo service" << (enabled ? "enabled" : "disabled");
}

void LocalEchoService::onEchoTimerTimeout()
{
    if (m_pendingContent.isEmpty()) {
        return;
    }
    
    // Create echo message
    core::Message echoMessage;
    echoMessage.setId(QString::number(QDateTime::currentMSecsSinceEpoch()));
    echoMessage.setFromUserId(getEchoBotId());
    echoMessage.setToUserId(core::UserProfile::instance().userId());
    echoMessage.setTimestamp(QDateTime::currentDateTime());
    
    // Echo back with a prefix
    QString echoContent = QString("Echo: %1").arg(m_pendingContent);
    echoMessage.setContent(echoContent);
    
    qInfo() << "[LocalEchoService] Echoing message:" << echoContent.left(50);
    
    // Emit the echo message
    emit messageReceived(echoMessage);
    
    // Clear pending content
    m_pendingContent.clear();
}

} // namespace services
} // namespace flykylin
