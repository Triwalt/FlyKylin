/**
 * @file ConfigManager.h
 * @brief 配置文件管理器
 * @author FlyKylin Team
 * @date 2025-11-19
 * 
 * 单例模式的配置管理器，负责用户配置的持久化存储和加载。
 * 支持UUID生成、MAC地址获取、配置文件备份等功能。
 * 
 * @see US-004 UUID身份持久化
 * @see ADR-001 UUID身份持久化方案
 */

#ifndef FLYKYLIN_CORE_CONFIG_CONFIGMANAGER_H
#define FLYKYLIN_CORE_CONFIG_CONFIGMANAGER_H

#include "UserProfile.h"
#include <QObject>
#include <QString>
#include <QMutex>
#include <memory>

namespace FlyKylin {
namespace Core {
namespace Config {

/**
 * @class ConfigManager
 * @brief 配置文件管理器（单例）
 * 
 * 负责用户配置的加载、保存和管理。
 * 首次启动时自动生成UUID和默认配置。
 * 配置文件存储在用户目录下。
 */
class ConfigManager : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief 获取单例实例
     * @return ConfigManager* 单例指针
     */
    static ConfigManager* instance();
    
    /**
     * @brief 加载配置文件
     * @return bool 加载是否成功
     */
    bool loadConfig();
    
    /**
     * @brief 保存配置文件
     * @return bool 保存是否成功
     */
    bool saveConfig();
    
    /**
     * @brief 获取用户配置
     * @return flykylin::core::UserProfile& 用户配置对象引用
     * @deprecated Use flykylin::core::UserProfile::instance() instead
     */
    const flykylin::core::UserProfile& userProfile() const;
    
    /**
     * @brief 设置用户配置
     * @param profile 用户配置对象
     * @deprecated Use flykylin::core::UserProfile::instance() methods instead
     */
    void setUserProfile(const flykylin::core::UserProfile& profile);
    
    /**
     * @brief 获取配置文件路径
     * @return QString 配置文件完整路径
     */
    QString configFilePath() const;
    
    /**
     * @brief 获取配置目录路径
     * @return QString 配置目录路径
     */
    QString configDir() const;

signals:
    /**
     * @brief 配置变更信号
     */
    void configChanged();
    
    /**
     * @brief 配置加载失败信号
     * @param error 错误信息
     */
    void configLoadFailed(const QString& error);
    
    /**
     * @brief 配置保存失败信号
     * @param error 错误信息
     */
    void configSaveFailed(const QString& error);

private:
    // 私有构造函数（单例模式）
    explicit ConfigManager(QObject* parent = nullptr);
    ~ConfigManager() override = default;
    
    // 禁止拷贝和赋值
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    /**
     * @brief 初始化默认配置
     */
    void initDefaultConfig();
    
    /**
     * @brief 生成新的UUID
     * @return QString UUID字符串
     */
    QString generateUuid();
    
    /**
     * @brief 获取系统MAC地址
     * @return QString MAC地址（格式：XX:XX:XX:XX:XX:XX）
     */
    QString getMacAddress();
    
    /**
     * @brief 获取系统主机名
     * @return QString 主机名
     */
    QString getHostName();
    
    /**
     * @brief 获取默认用户名
     * @return QString 用户名（基于系统用户）
     */
    QString getDefaultUserName();
    
    /**
     * @brief 确保配置目录存在
     * @return bool 是否成功
     */
    bool ensureConfigDirExists();
    
    /**
     * @brief 创建配置文件备份
     * @return bool 是否成功
     */
    bool createBackup();
    
    /**
     * @brief 从备份恢复配置
     * @return bool 是否成功
     */
    bool restoreFromBackup();

private:
    static ConfigManager* s_instance;  ///< 单例实例
    static QMutex s_mutex;             ///< 实例创建互斥锁
    
    // NOTE: UserProfile is now a singleton, ConfigManager is deprecated
    // UserProfile m_userProfile;      ///< DEPRECATED: Use UserProfile::instance()
    QString m_configPath;              ///< 配置文件路径
    mutable QMutex m_profileMutex;     ///< 配置访问互斥锁（已废弃）
};

} // namespace Config
} // namespace Core
} // namespace FlyKylin

#endif // FLYKYLIN_CORE_CONFIG_CONFIGMANAGER_H
