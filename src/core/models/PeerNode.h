/**
 * @file PeerNode.h
 * @brief P2P节点信息数据模型
 * @author FlyKylin Team
 * @date 2024-11-18
 */

#pragma once

#include <QString>
#include <QDateTime>
#include <QHostAddress>

namespace flykylin {
namespace core {

/**
 * @class PeerNode
 * @brief 表示一个P2P网络中的节点
 * 
 * 包含节点的基本信息和连接状态
 */
class PeerNode {
public:
    /**
     * @brief 默认构造函数
     */
    PeerNode() = default;

    /**
     * @brief 构造函数
     * @param userId 用户ID（通常是IP地址）
     * @param userName 用户名
     * @param hostName 主机名
     * @param ipAddress IP地址
     * @param tcpPort TCP监听端口
     */
    PeerNode(const QString& userId,
             const QString& userName,
             const QString& hostName,
             const QHostAddress& ipAddress,
             quint16 tcpPort);

    // Getters
    QString userId() const { return m_userId; }
    QString userName() const { return m_userName; }
    QString hostName() const { return m_hostName; }
    QHostAddress ipAddress() const { return m_ipAddress; }
    quint16 tcpPort() const { return m_tcpPort; }
    QDateTime lastSeen() const { return m_lastSeen; }
    bool isOnline() const { return m_isOnline; }

    // Setters
    void setUserId(const QString& userId) { m_userId = userId; }
    void setUserName(const QString& userName) { m_userName = userName; }
    void setHostName(const QString& hostName) { m_hostName = hostName; }
    void setIpAddress(const QString& ipAddress) { m_ipAddress = QHostAddress(ipAddress); }
    void setTcpPort(quint16 port) { m_tcpPort = port; }
    void setLastSeen(const QDateTime& time) { m_lastSeen = time; }
    void setOnline(bool online) { m_isOnline = online; }

    /**
     * @brief 更新最后心跳时间为当前时间
     */
    void updateLastSeen();

    /**
     * @brief 检查节点是否超时
     * @param timeoutSeconds 超时时间（秒）
     * @return 如果超时返回true
     */
    bool isTimedOut(int timeoutSeconds = 30) const;

    // 比较运算符
    bool operator==(const PeerNode& other) const;
    bool operator!=(const PeerNode& other) const;

private:
    QString m_userId;           ///< 用户ID
    QString m_userName;         ///< 用户名
    QString m_hostName;         ///< 主机名
    QHostAddress m_ipAddress;   ///< IP地址
    quint16 m_tcpPort{0};       ///< TCP端口
    QDateTime m_lastSeen;       ///< 最后心跳时间
    bool m_isOnline{true};      ///< 是否在线
};

} // namespace core
} // namespace flykylin
