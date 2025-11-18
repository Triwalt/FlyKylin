/**
 * @file UserProfile.cpp
 * @brief 用户配置数据模型实现
 * @author FlyKylin Team
 * @date 2025-11-19
 */

#include "UserProfile.h"
#include <QUuid>
#include <QRegularExpression>
#include <QJsonDocument>

namespace FlyKylin {
namespace Core {
namespace Config {

UserProfile::UserProfile()
    : m_createdAt(QDateTime::currentSecsSinceEpoch())
    , m_updatedAt(QDateTime::currentSecsSinceEpoch())
{
}

UserProfile::UserProfile(const QString& uuid, const QString& userName)
    : m_uuid(uuid)
    , m_userName(userName)
    , m_createdAt(QDateTime::currentSecsSinceEpoch())
    , m_updatedAt(QDateTime::currentSecsSinceEpoch())
{
}

QJsonObject UserProfile::toJson() const
{
    QJsonObject json;
    json["uuid"] = m_uuid;
    json["user_name"] = m_userName;
    json["host_name"] = m_hostName;
    json["avatar_path"] = m_avatarPath;
    json["mac_address"] = m_macAddress;
    json["created_at"] = m_createdAt;
    json["updated_at"] = m_updatedAt;
    return json;
}

UserProfile UserProfile::fromJson(const QJsonObject& json)
{
    UserProfile profile;
    profile.m_uuid = json["uuid"].toString();
    profile.m_userName = json["user_name"].toString();
    profile.m_hostName = json["host_name"].toString();
    profile.m_avatarPath = json["avatar_path"].toString();
    profile.m_macAddress = json["mac_address"].toString();
    profile.m_createdAt = json["created_at"].toInteger();
    profile.m_updatedAt = json["updated_at"].toInteger();
    return profile;
}

bool UserProfile::isValidUuid(const QString& uuid)
{
    if (uuid.isEmpty()) {
        return false;
    }
    
    // UUID格式: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    // 例如: a1b2c3d4-e5f6-4012-8901-9abcdef01234
    QRegularExpression uuidRegex(
        R"(^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$)",
        QRegularExpression::CaseInsensitiveOption
    );
    
    return uuidRegex.match(uuid).hasMatch();
}

void UserProfile::touch()
{
    m_updatedAt = QDateTime::currentSecsSinceEpoch();
}

bool UserProfile::isValid() const
{
    return !m_uuid.isEmpty() && !m_userName.isEmpty();
}

} // namespace Config
} // namespace Core
} // namespace FlyKylin
