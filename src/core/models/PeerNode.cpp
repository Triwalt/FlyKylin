/**
 * @file PeerNode.cpp
 * @brief P2P节点信息数据模型实现
 */

#include "PeerNode.h"

namespace flykylin {
namespace core {

PeerNode::PeerNode(const QString& userId,
                   const QString& userName,
                   const QString& hostName,
                   const QHostAddress& ipAddress,
                   quint16 tcpPort)
    : m_userId(userId)
    , m_userName(userName)
    , m_hostName(hostName)
    , m_ipAddress(ipAddress)
    , m_tcpPort(tcpPort)
    , m_lastSeen(QDateTime::currentDateTime())
    , m_isOnline(true)
{
}

void PeerNode::updateLastSeen()
{
    m_lastSeen = QDateTime::currentDateTime();
    m_isOnline = true;
}

bool PeerNode::isTimedOut(int timeoutSeconds) const
{
    if (!m_lastSeen.isValid()) {
        return true;
    }
    
    qint64 elapsedSeconds = m_lastSeen.secsTo(QDateTime::currentDateTime());
    return elapsedSeconds > timeoutSeconds;
}

bool PeerNode::operator==(const PeerNode& other) const
{
    return m_userId == other.m_userId;
}

bool PeerNode::operator!=(const PeerNode& other) const
{
    return !(*this == other);
}

} // namespace core
} // namespace flykylin
