/**
 * @file GroupChatManager.cpp
 * @brief 群聊状态管理服务实现
 * @author FlyKylin Team
 * @date 2025-12-06
 */

#include "GroupChatManager.h"
#include <QMutexLocker>
#include <QDebug>

namespace flykylin {
namespace core {
namespace services {

GroupChatManager* GroupChatManager::s_instance = nullptr;

GroupChatManager* GroupChatManager::instance() {
    if (!s_instance) {
        s_instance = new GroupChatManager();
    }
    return s_instance;
}

GroupChatManager::GroupChatManager(QObject* parent)
    : QObject(parent)
{
    qInfo() << "[GroupChatManager] Created";
}

void GroupChatManager::registerGroup(const QString& groupId,
                                     const QStringList& memberIds,
                                     const QString& ownerId)
{
    if (groupId.isEmpty()) {
        qWarning() << "[GroupChatManager] registerGroup: empty groupId";
        return;
    }

    QMutexLocker locker(&m_mutex);

    // 规范化成员列表（去重、去空）
    QStringList normalizedMembers;
    normalizedMembers.reserve(memberIds.size());
    for (const QString& m : memberIds) {
        if (m.isEmpty()) {
            continue;
        }
        if (!normalizedMembers.contains(m)) {
            normalizedMembers.append(m);
        }
    }

    GroupMeta& meta = m_groups[groupId];

    // 更新群主（只有在未设置或显式传入时）
    if (!ownerId.isEmpty()) {
        if (meta.ownerId.isEmpty()) {
            meta.ownerId = ownerId;
        } else if (meta.ownerId != ownerId) {
            qWarning() << "[GroupChatManager] registerGroup: ownerId mismatch for group"
                       << groupId << "existing" << meta.ownerId << "new" << ownerId;
        }
    }

    // 合并成员列表
    for (const QString& m : normalizedMembers) {
        if (!meta.members.contains(m)) {
            meta.members.append(m);
            emit memberJoined(groupId, m);
        }
    }

    qInfo() << "[GroupChatManager] Registered group" << groupId 
            << "owner=" << meta.ownerId
            << "members=" << meta.members;

    emit groupUpdated(groupId);
}

QStringList GroupChatManager::getGroupMembers(const QString& groupId) const {
    QMutexLocker locker(&m_mutex);
    
    auto it = m_groups.constFind(groupId);
    if (it == m_groups.constEnd()) {
        return QStringList();
    }
    return it->members;
}

QString GroupChatManager::getGroupOwner(const QString& groupId) const {
    QMutexLocker locker(&m_mutex);
    
    auto it = m_groups.constFind(groupId);
    if (it == m_groups.constEnd()) {
        return QString();
    }
    return it->ownerId;
}

bool GroupChatManager::isGroupMember(const QString& groupId, const QString& userId) const {
    QMutexLocker locker(&m_mutex);
    
    auto it = m_groups.constFind(groupId);
    if (it == m_groups.constEnd()) {
        return false;
    }
    return it->members.contains(userId) || it->ownerId == userId;
}

bool GroupChatManager::hasGroup(const QString& groupId) const {
    QMutexLocker locker(&m_mutex);
    return m_groups.contains(groupId);
}

bool GroupChatManager::addMember(const QString& groupId, const QString& userId) {
    if (groupId.isEmpty() || userId.isEmpty()) {
        return false;
    }

    QMutexLocker locker(&m_mutex);
    
    auto it = m_groups.find(groupId);
    if (it == m_groups.end()) {
        return false;
    }

    if (it->members.contains(userId) || it->ownerId == userId) {
        return false; // 已存在
    }

    it->members.append(userId);
    
    locker.unlock();
    emit memberJoined(groupId, userId);
    emit groupUpdated(groupId);
    
    return true;
}

bool GroupChatManager::removeMember(const QString& groupId, const QString& userId) {
    if (groupId.isEmpty() || userId.isEmpty()) {
        return false;
    }

    QMutexLocker locker(&m_mutex);
    
    auto it = m_groups.find(groupId);
    if (it == m_groups.end()) {
        return false;
    }

    bool removed = it->members.removeOne(userId);
    
    if (removed) {
        locker.unlock();
        emit memberLeft(groupId, userId);
        emit groupUpdated(groupId);
    }
    
    return removed;
}

void GroupChatManager::removeGroup(const QString& groupId) {
    if (groupId.isEmpty()) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    
    if (m_groups.remove(groupId) > 0) {
        locker.unlock();
        qInfo() << "[GroupChatManager] Removed group" << groupId;
        emit groupRemoved(groupId);
    }
}

QStringList GroupChatManager::getMessageTargets(const QString& groupId,
                                                const QString& localUserId,
                                                const QString& excludeUserId) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_groups.constFind(groupId);
    if (it == m_groups.constEnd()) {
        return QStringList();
    }

    const GroupMeta& meta = it.value();
    QStringList targets;

    if (meta.ownerId.isEmpty()) {
        // 无群主模式：发给所有成员
        for (const QString& m : meta.members) {
            if (m.isEmpty() || m == localUserId || m == excludeUserId) {
                continue;
            }
            if (!targets.contains(m)) {
                targets.append(m);
            }
        }
    } else if (meta.ownerId == localUserId) {
        // 群主发消息：发给所有成员
        for (const QString& m : meta.members) {
            if (m.isEmpty() || m == localUserId || m == excludeUserId) {
                continue;
            }
            if (!targets.contains(m)) {
                targets.append(m);
            }
        }
    } else {
        // 成员发消息：只发给群主
        if (!meta.ownerId.isEmpty() && 
            meta.ownerId != excludeUserId &&
            meta.members.contains(meta.ownerId)) {
            targets.append(meta.ownerId);
        }
    }

    return targets;
}

QStringList GroupChatManager::getRelayTargets(const QString& groupId,
                                              const QString& localUserId,
                                              const QString& fromUserId,
                                              const QString& toUserId) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_groups.constFind(groupId);
    if (it == m_groups.constEnd()) {
        return QStringList();
    }

    const GroupMeta& meta = it.value();

    // 只有群主才能中继消息
    if (meta.ownerId.isEmpty() || meta.ownerId != localUserId) {
        return QStringList();
    }

    QStringList relayTargets;
    for (const QString& memberId : meta.members) {
        if (memberId.isEmpty()) {
            continue;
        }
        // 排除自己、发送者、原接收者
        if (memberId == localUserId ||
            memberId == fromUserId ||
            memberId == toUserId) {
            continue;
        }
        if (!relayTargets.contains(memberId)) {
            relayTargets.append(memberId);
        }
    }

    return relayTargets;
}

} // namespace services
} // namespace core
} // namespace flykylin
