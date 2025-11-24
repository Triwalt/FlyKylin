/**
 * @file UserProfile.cpp
 * @brief 用户配置单例实现
 * @author FlyKylin Team
 * @date 2024-11-20
 */

#include "UserProfile.h"
#include <QUuid>
#include <QSettings>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QDebug>

namespace flykylin {
namespace core {

UserProfile& UserProfile::instance() {
    static UserProfile instance;
    return instance;
}

UserProfile::UserProfile()
    : m_createdAt(0)
    , m_updatedAt(0)
{
    load();
    // 默认情况下，运行时有效ID与持久化UUID一致
    m_effectiveUserId = m_uuid;
    qInfo() << "[UserProfile] Initialized for user" << m_uuid;
}

void UserProfile::load() {
    QSettings settings("FlyKylin", "FlyKylin");
    
    // 加载UUID（如果不存在则生成新的）
    m_uuid = settings.value("uuid").toString();
    if (m_uuid.isEmpty()) {
        // 首次运行，生成新UUID
        m_uuid = generateUuid();
        m_createdAt = QDateTime::currentSecsSinceEpoch();
        m_updatedAt = m_createdAt;
        
        qInfo() << "[UserProfile] First run, generated new UUID:" << m_uuid;
    } else {
        m_createdAt = settings.value("createdAt", 0).toLongLong();
        m_updatedAt = settings.value("updatedAt", 0).toLongLong();
        
        qInfo() << "[UserProfile] Loaded existing UUID:" << m_uuid;
    }
    
    // 加载其他配置（使用默认值），并修复历史上保存为空字符串的情况
    m_hostName = settings.value("hostName").toString();
    if (m_hostName.isEmpty()) {
        m_hostName = getSystemHostName();
    }

    m_userName = settings.value("userName").toString();
    if (m_userName.isEmpty()) {
        m_userName = m_hostName;
    }

    m_avatarPath = settings.value("avatarPath").toString();

    m_macAddress = settings.value("macAddress").toString();
    if (m_macAddress.isEmpty()) {
        m_macAddress = getMacAddress();
    }
    
    qDebug() << "[UserProfile] User:" << m_userName 
             << "Host:" << m_hostName 
             << "MAC:" << m_macAddress;

    // 确保首次运行或修复后的配置被持久化
    save();
}

void UserProfile::save() {
    QSettings settings("FlyKylin", "FlyKylin");
    
    settings.setValue("uuid", m_uuid);
    settings.setValue("userName", m_userName);
    settings.setValue("hostName", m_hostName);
    settings.setValue("avatarPath", m_avatarPath);
    settings.setValue("macAddress", m_macAddress);
    settings.setValue("createdAt", m_createdAt);
    settings.setValue("updatedAt", m_updatedAt);
    
    settings.sync();
    
    if (settings.status() != QSettings::NoError) {
        qCritical() << "[UserProfile] Failed to save settings";
    } else {
        qDebug() << "[UserProfile] Settings saved successfully";
    }
}

void UserProfile::setUserName(const QString& userName) {
    if (m_userName != userName) {
        m_userName = userName;
        m_updatedAt = QDateTime::currentSecsSinceEpoch();
        save();
        qInfo() << "[UserProfile] User name updated to:" << userName;
    }
}

void UserProfile::setHostName(const QString& hostName) {
    if (m_hostName != hostName) {
        m_hostName = hostName;
        m_updatedAt = QDateTime::currentSecsSinceEpoch();
        save();
    }
}

void UserProfile::setAvatarPath(const QString& avatarPath) {
    if (m_avatarPath != avatarPath) {
        m_avatarPath = avatarPath;
        m_updatedAt = QDateTime::currentSecsSinceEpoch();
        save();
    }
}

void UserProfile::setMacAddress(const QString& macAddress) {
    if (m_macAddress != macAddress) {
        m_macAddress = macAddress;
        m_updatedAt = QDateTime::currentSecsSinceEpoch();
        save();
        qInfo() << "[UserProfile] MAC address updated to:" << macAddress;
    }
}

void UserProfile::setInstanceSuffix(const QString& suffix) {
    QString newId = m_uuid;
    if (!suffix.isEmpty()) {
        newId += suffix;
    }

    if (m_effectiveUserId == newId) {
        return;
    }

    m_effectiveUserId = newId;
    qInfo() << "[UserProfile] Instance suffix set to:" << suffix
            << "effective userId:" << m_effectiveUserId;
}

bool UserProfile::isValid() const {
    return !m_uuid.isEmpty() && 
           !m_userName.isEmpty() &&
           QUuid(m_uuid).isNull() == false;  // 验证UUID格式
}

QString UserProfile::generateUuid() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString UserProfile::getSystemHostName() {
    QString hostName = QHostInfo::localHostName();
    if (hostName.isEmpty()) {
        hostName = "UnknownHost";
    }
    return hostName;
}

QString UserProfile::getMacAddress() {
    // 获取第一个非回环网络接口的MAC地址
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    
    for (const QNetworkInterface& interface : interfaces) {
        // 跳过回环接口和未激活的接口
        if (interface.flags().testFlag(QNetworkInterface::IsLoopBack) ||
            !interface.flags().testFlag(QNetworkInterface::IsUp)) {
            continue;
        }
        
        QString mac = interface.hardwareAddress();
        if (!mac.isEmpty()) {
            return mac;
        }
    }
    
    return "00:00:00:00:00:00";  // 默认值
}

} // namespace core
} // namespace flykylin
