/**
 * @file UserProfile.h
 * @brief 用户配置数据模型
 * @author FlyKylin Team
 * @date 2025-11-19
 * 
 * 用户配置信息的数据结构，包含UUID、用户名、MAC地址等信息。
 * 支持JSON序列化和反序列化。
 * 
 * @see US-004 UUID身份持久化
 */

#ifndef FLYKYLIN_CORE_CONFIG_USERPROFILE_H
#define FLYKYLIN_CORE_CONFIG_USERPROFILE_H

#include <QString>
#include <QJsonObject>
#include <QDateTime>

namespace FlyKylin {
namespace Core {
namespace Config {

/**
 * @class UserProfile
 * @brief 用户配置数据模型
 * 
 * 存储用户的基本信息和持久化标识。
 * UUID作为用户的全局唯一标识，不随IP变化而改变。
 */
class UserProfile {
public:
    /**
     * @brief 默认构造函数
     */
    UserProfile();
    
    /**
     * @brief 带参数构造函数
     * @param uuid 用户UUID
     * @param userName 用户名
     */
    UserProfile(const QString& uuid, const QString& userName);
    
    // Getters
    QString uuid() const { return m_uuid; }
    QString userName() const { return m_userName; }
    QString hostName() const { return m_hostName; }
    QString avatarPath() const { return m_avatarPath; }
    QString macAddress() const { return m_macAddress; }
    qint64 createdAt() const { return m_createdAt; }
    qint64 updatedAt() const { return m_updatedAt; }
    
    // Setters
    void setUuid(const QString& uuid) { m_uuid = uuid; }
    void setUserName(const QString& userName) { m_userName = userName; }
    void setHostName(const QString& hostName) { m_hostName = hostName; }
    void setAvatarPath(const QString& avatarPath) { m_avatarPath = avatarPath; }
    void setMacAddress(const QString& macAddress) { m_macAddress = macAddress; }
    void setCreatedAt(qint64 timestamp) { m_createdAt = timestamp; }
    void setUpdatedAt(qint64 timestamp) { m_updatedAt = timestamp; }
    
    /**
     * @brief 序列化为JSON对象
     * @return QJsonObject JSON对象
     */
    QJsonObject toJson() const;
    
    /**
     * @brief 从JSON对象反序列化
     * @param json JSON对象
     * @return UserProfile 用户配置对象
     */
    static UserProfile fromJson(const QJsonObject& json);
    
    /**
     * @brief 验证UUID格式
     * @param uuid UUID字符串
     * @return bool 是否有效
     */
    static bool isValidUuid(const QString& uuid);
    
    /**
     * @brief 更新时间戳为当前时间
     */
    void touch();
    
    /**
     * @brief 判断配置是否有效
     * @return bool UUID和用户名都不为空
     */
    bool isValid() const;

private:
    QString m_uuid;         ///< 用户UUID（全局唯一标识）
    QString m_userName;     ///< 用户名
    QString m_hostName;     ///< 主机名
    QString m_avatarPath;   ///< 头像路径
    QString m_macAddress;   ///< MAC地址（辅助识别）
    qint64 m_createdAt;     ///< 创建时间（Unix时间戳）
    qint64 m_updatedAt;     ///< 更新时间（Unix时间戳）
};

} // namespace Config
} // namespace Core
} // namespace FlyKylin

#endif // FLYKYLIN_CORE_CONFIG_USERPROFILE_H
