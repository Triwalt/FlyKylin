/**
 * @file ConfigManager.cpp
 * @brief 配置文件管理器实现
 * @author FlyKylin Team
 * @date 2025-11-19
 */

#include "ConfigManager.h"
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QMutexLocker>
#include <QDebug>

namespace FlyKylin {
namespace Core {
namespace Config {

// 静态成员初始化
ConfigManager* ConfigManager::s_instance = nullptr;
QMutex ConfigManager::s_mutex;

// 配置文件名常量
constexpr const char* kConfigFileName = "user_profile.json";
constexpr const char* kBackupSuffix = ".bak";

ConfigManager* ConfigManager::instance()
{
    if (s_instance == nullptr) {
        QMutexLocker locker(&s_mutex);
        if (s_instance == nullptr) {
            s_instance = new ConfigManager();
        }
    }
    return s_instance;
}

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
{
    // 获取配置文件路径
    QString appDataPath = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    m_configPath = appDataPath + QDir::separator() + kConfigFileName;
    
    qDebug() << "ConfigManager initialized, config path:" << m_configPath;
}

bool ConfigManager::loadConfig()
{
    QMutexLocker locker(&m_profileMutex);
    
    // 确保配置目录存在
    if (!ensureConfigDirExists()) {
        qCritical() << "Failed to create config directory";
        emit configLoadFailed("Failed to create config directory");
        return false;
    }
    
    QFile configFile(m_configPath);
    
    // 配置文件不存在，初始化默认配置
    if (!configFile.exists()) {
        qInfo() << "Config file not found, creating default config";
        initDefaultConfig();
        return saveConfig();  // 递归调用，但不会再次加锁
    }
    
    // 打开配置文件
    if (!configFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open config file:" << configFile.errorString();
        
        // 尝试从备份恢复
        if (restoreFromBackup()) {
            qInfo() << "Restored config from backup";
            return loadConfig();  // 重新加载
        }
        
        // 恢复失败，初始化默认配置
        qWarning() << "Backup restore failed, initializing default config";
        initDefaultConfig();
        emit configLoadFailed("Config file corrupted, using default");
        return false;
    }
    
    // 读取并解析JSON
    QByteArray jsonData = configFile.readAll();
    configFile.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qCritical() << "JSON parse error:" << parseError.errorString();
        
        // 尝试从备份恢复
        if (restoreFromBackup()) {
            qInfo() << "Restored config from backup after parse error";
            return loadConfig();
        }
        
        // 恢复失败，初始化默认配置
        qWarning() << "Backup restore failed, initializing default config";
        initDefaultConfig();
        emit configLoadFailed("JSON parse error: " + parseError.errorString());
        return false;
    }
    
    if (!doc.isObject()) {
        qCritical() << "JSON root is not an object";
        initDefaultConfig();
        emit configLoadFailed("Invalid JSON structure");
        return false;
    }
    
    // 提取用户配置
    QJsonObject root = doc.object();
    if (!root.contains("user_profile")) {
        qWarning() << "Missing user_profile field";
        initDefaultConfig();
        emit configLoadFailed("Missing user_profile field");
        return false;
    }
    
    // NOTE: ConfigManager is deprecated, UserProfile handles its own loading
    qInfo() << "[ConfigManager] Config loading delegated to UserProfile singleton";
    
    // Verify UserProfile is valid
    if (!flykylin::core::UserProfile::instance().isValid()) {
        qWarning() << "[ConfigManager] UserProfile singleton is invalid";
        initDefaultConfig();
        emit configLoadFailed("Invalid profile data");
        return false;
    }
    
    qInfo() << "[ConfigManager] Config loaded via UserProfile, UUID:" << flykylin::core::UserProfile::instance().userId();
    emit configChanged();
    return true;
}

bool ConfigManager::saveConfig()
{
    QMutexLocker locker(&m_profileMutex);
    
    // 确保配置目录存在
    if (!ensureConfigDirExists()) {
        qCritical() << "Failed to create config directory";
        emit configSaveFailed("Failed to create config directory");
        return false;
    }
    
    // 创建备份
    if (QFile::exists(m_configPath)) {
        createBackup();
    }
    
    // NOTE: ConfigManager is deprecated, UserProfile handles its own saving
    qInfo() << "[ConfigManager] Config saving delegated to UserProfile singleton";
    
    // UserProfile saves automatically, this is a no-op
    QJsonObject root;
    // root["user_profile"] = m_userProfile.toJson();  // DEPRECATED
    
    // 添加元数据
    root["version"] = "1.0";
    root["last_modified"] = QDateTime::currentSecsSinceEpoch();
    
    // 写入文件
    QFile configFile(m_configPath);
    if (!configFile.open(QIODevice::WriteOnly)) {
        qCritical() << "Failed to open config file for writing:" 
                    << configFile.errorString();
        emit configSaveFailed("Failed to open file: " + configFile.errorString());
        return false;
    }
    
    QJsonDocument doc(root);
    qint64 bytesWritten = configFile.write(doc.toJson(QJsonDocument::Indented));
    configFile.close();
    
    if (bytesWritten == -1) {
        qCritical() << "Failed to write config file";
        emit configSaveFailed("Failed to write file");
        return false;
    }
    
    qInfo() << "Config saved successfully";
    emit configChanged();
    return true;
}

const flykylin::core::UserProfile& ConfigManager::userProfile() const
{
    // DEPRECATED: Return UserProfile::instance() reference to maintain API compatibility
    // Callers should use UserProfile::instance() directly
    qWarning() << "[ConfigManager] userProfile() is deprecated, use UserProfile::instance() directly";
    
    return flykylin::core::UserProfile::instance();
}

void ConfigManager::setUserProfile(const flykylin::core::UserProfile& profile)
{
    // DEPRECATED: ConfigManager no longer manages UserProfile
    // Use UserProfile::instance().setUserName() etc. instead
    qWarning() << "[ConfigManager] setUserProfile() is deprecated and does nothing";
    qWarning() << "[ConfigManager] Use UserProfile::instance() methods instead";
    emit configChanged();
}

QString ConfigManager::configFilePath() const
{
    return m_configPath;
}

QString ConfigManager::configDir() const
{
    return QFileInfo(m_configPath).absolutePath();
}

void ConfigManager::initDefaultConfig()
{
    qInfo() << "[ConfigManager] Initializing default configuration";
    qInfo() << "[ConfigManager] NOTE: ConfigManager is deprecated, UserProfile singleton manages config";
    
    // UserProfile singleton已经自动初始化和管理配置
    // ConfigManager现在是legacy代码，保留兼容性
    
    qInfo() << "[ConfigManager] Default config handled by UserProfile singleton";
}

QString ConfigManager::generateUuid()
{
    QUuid uuid = QUuid::createUuid();
    QString uuidStr = uuid.toString(QUuid::WithoutBraces);
    qDebug() << "Generated new UUID:" << uuidStr;
    return uuidStr;
}

QString ConfigManager::getMacAddress()
{
    // 获取所有网络接口
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    
    for (const auto& interface : interfaces) {
        // 跳过回环接口
        if (interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            continue;
        }
        
        // 跳过未激活的接口
        if (!interface.flags().testFlag(QNetworkInterface::IsUp)) {
            continue;
        }
        
        // 获取MAC地址
        QString mac = interface.hardwareAddress();
        if (!mac.isEmpty()) {
            qDebug() << "Found MAC address:" << mac;
            return mac;
        }
    }
    
    qWarning() << "No MAC address found";
    return QString();
}

QString ConfigManager::getHostName()
{
    QString hostName = QHostInfo::localHostName();
    qDebug() << "Host name:" << hostName;
    return hostName;
}

QString ConfigManager::getDefaultUserName()
{
    // 尝试获取系统用户名
    QString userName = qgetenv("USER");
    if (userName.isEmpty()) {
        userName = qgetenv("USERNAME");  // Windows
    }
    
    if (userName.isEmpty()) {
        userName = "User";  // 默认值
    }
    
    qDebug() << "Default user name:" << userName;
    return userName;
}

bool ConfigManager::ensureConfigDirExists()
{
    QDir configDir(QFileInfo(m_configPath).absolutePath());
    if (!configDir.exists()) {
        if (!configDir.mkpath(".")) {
            qCritical() << "Failed to create config directory:" 
                        << configDir.absolutePath();
            return false;
        }
        qInfo() << "Created config directory:" << configDir.absolutePath();
    }
    return true;
}

bool ConfigManager::createBackup()
{
    QString backupPath = m_configPath + kBackupSuffix;
    
    // 删除旧备份
    if (QFile::exists(backupPath)) {
        QFile::remove(backupPath);
    }
    
    // 复制当前配置文件为备份
    if (!QFile::copy(m_configPath, backupPath)) {
        qWarning() << "Failed to create backup";
        return false;
    }
    
    qDebug() << "Created config backup:" << backupPath;
    return true;
}

bool ConfigManager::restoreFromBackup()
{
    QString backupPath = m_configPath + kBackupSuffix;
    
    if (!QFile::exists(backupPath)) {
        qWarning() << "Backup file not found";
        return false;
    }
    
    // 删除损坏的配置文件
    if (QFile::exists(m_configPath)) {
        QFile::remove(m_configPath);
    }
    
    // 从备份恢复
    if (!QFile::copy(backupPath, m_configPath)) {
        qCritical() << "Failed to restore from backup";
        return false;
    }
    
    qInfo() << "Restored config from backup";
    return true;
}

} // namespace Config
} // namespace Core
} // namespace FlyKylin
