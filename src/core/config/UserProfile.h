/**
 * @file UserProfile.h
 * @brief 用户配置单例
 * @author FlyKylin Team
 * @date 2024-11-20
 * 
 * 用户配置信息的单例类，管理用户UUID、用户名等持久化信息。
 * 使用QSettings实现配置持久化，UUID一次生成永久保存。
 * 
 * @note 线程安全的单例实现（C++11 magic static）
 * @see TD-009 UserProfile单例实现
 */

#pragma once

#include <QString>
#include <QJsonObject>
#include <QDateTime>
#include <memory>

namespace flykylin {
namespace core {

/**
 * @class UserProfile
 * @brief 用户配置单例
 * 
 * 管理用户的基本信息和持久化标识。
 * UUID作为用户的全局唯一标识，不随IP变化而改变。
 * 
 * 使用单例模式确保全局唯一实例，配置自动持久化到QSettings。
 * 
 * 使用示例：
 * @code
 * QString userId = UserProfile::instance().userId();
 * QString userName = UserProfile::instance().userName();
 * @endcode
 */
class UserProfile {
public:
    /**
     * @brief 获取单例实例
     * @return UserProfile& 单例引用
     * @note 线程安全（C++11 magic static）
     */
    static UserProfile& instance();
    
    // 删除拷贝构造和赋值运算符
    UserProfile(const UserProfile&) = delete;
    UserProfile& operator=(const UserProfile&) = delete;
    
    // Getters
    /**
     * @brief 获取当前实例的用户ID
     *
     * 默认情况下等于持久化UUID；
     * 调用 setInstanceSuffix() 后，会在UUID后附加实例后缀，
     * 使得同一账号在多进程/多端口运行时具有不同的实例ID。
     */
    QString userId() const { return m_effectiveUserId; }
    
    /**
     * @brief 获取用户名
     * @return 用户名，默认为主机名
     */
    QString userName() const { return m_userName; }
    
    QString hostName() const { return m_hostName; }
    QString avatarPath() const { return m_avatarPath; }
    QString macAddress() const { return m_macAddress; }
    qint64 createdAt() const { return m_createdAt; }
    qint64 updatedAt() const { return m_updatedAt; }
    
    // Setters（修改后自动保存）
    /**
     * @brief 设置用户名并保存
     * @param userName 新用户名
     */
    void setUserName(const QString& userName);
    
    void setHostName(const QString& hostName);
    void setAvatarPath(const QString& avatarPath);
    void setMacAddress(const QString& macAddress);

    /**
     * @brief 为当前进程设置实例后缀（仅影响运行时ID，不写入配置）
     *
     * 例如可传入TCP监听端口，生成形如 "<uuid>:45679" 的实例ID，
     * 用于区分同一设备上的多个进程实例。
     */
    void setInstanceSuffix(const QString& suffix);
    
    /**
     * @brief 判断配置是否有效
     * @return bool UUID和用户名都不为空
     */
    bool isValid() const;
    
private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    UserProfile();
    
    /**
     * @brief 析构函数
     */
    ~UserProfile() = default;
    
    /**
     * @brief 从QSettings加载配置
     */
    void load();
    
    /**
     * @brief 保存配置到QSettings
     */
    void save();
    
    /**
     * @brief 生成新的UUID
     * @return QString 新生成的UUID
     */
    static QString generateUuid();
    
    /**
     * @brief 获取系统主机名
     * @return QString 主机名
     */
    static QString getSystemHostName();
    
    /**
     * @brief 获取MAC地址
     * @return QString MAC地址
     */
    static QString getMacAddress();

private:
    QString m_uuid;         ///< 持久化UUID（基础ID，不含实例后缀）
    QString m_effectiveUserId; ///< 运行时有效用户ID（可能包含实例后缀）
    QString m_userName;     ///< 用户名
    QString m_hostName;     ///< 主机名
    QString m_avatarPath;   ///< 头像路径
    QString m_macAddress;   ///< MAC地址（辅助识别）
    qint64 m_createdAt;     ///< 创建时间（Unix时间戳）
    qint64 m_updatedAt;     ///< 更新时间（Unix时间戳）
};

} // namespace core
} // namespace flykylin
