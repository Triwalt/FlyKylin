/**
 * @file GroupChatManager.h
 * @brief 群聊状态管理服务
 * @author FlyKylin Team
 * @date 2025-12-06
 * 
 * 职责:
 * - 管理群组元数据（成员、所有者）
 * - 处理群组注册和查询
 * - 与 QML groupsModel 保持同步
 * - 处理群消息路由逻辑
 * 
 * @see ChatViewModel - 使用此服务管理群聊状态
 */

#ifndef FLYKYLIN_CORE_SERVICES_GROUPCHATMANAGER_H
#define FLYKYLIN_CORE_SERVICES_GROUPCHATMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QMutex>

namespace flykylin {
namespace core {
namespace services {

/**
 * @brief 群聊状态管理服务（单例）
 */
class GroupChatManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static GroupChatManager* instance();

    /**
     * @brief 注册或更新群组信息
     * @param groupId 群组ID
     * @param memberIds 成员列表
     * @param ownerId 群主ID（可选，为空时不更新）
     */
    Q_INVOKABLE void registerGroup(const QString& groupId,
                                   const QStringList& memberIds,
                                   const QString& ownerId = QString());

    /**
     * @brief 获取群组成员列表
     * @param groupId 群组ID
     * @return 成员ID列表，群组不存在时返回空列表
     */
    Q_INVOKABLE QStringList getGroupMembers(const QString& groupId) const;

    /**
     * @brief 获取群主ID
     * @param groupId 群组ID
     * @return 群主ID，不存在时返回空字符串
     */
    Q_INVOKABLE QString getGroupOwner(const QString& groupId) const;

    /**
     * @brief 检查用户是否为群组成员
     * @param groupId 群组ID
     * @param userId 用户ID
     * @return true if user is a member
     */
    Q_INVOKABLE bool isGroupMember(const QString& groupId, const QString& userId) const;

    /**
     * @brief 检查群组是否存在
     * @param groupId 群组ID
     */
    Q_INVOKABLE bool hasGroup(const QString& groupId) const;

    /**
     * @brief 添加成员到群组
     * @param groupId 群组ID
     * @param userId 用户ID
     * @return true if added (false if already exists or group doesn't exist)
     */
    Q_INVOKABLE bool addMember(const QString& groupId, const QString& userId);

    /**
     * @brief 从群组移除成员
     * @param groupId 群组ID
     * @param userId 用户ID
     * @return true if removed
     */
    Q_INVOKABLE bool removeMember(const QString& groupId, const QString& userId);

    /**
     * @brief 删除群组
     * @param groupId 群组ID
     */
    Q_INVOKABLE void removeGroup(const QString& groupId);

    /**
     * @brief 获取消息转发目标列表
     * 
     * 根据群主/成员角色，确定消息应该发送给谁：
     * - 如果当前用户是群主，消息发给所有成员
     * - 如果当前用户是成员，消息只发给群主
     * 
     * @param groupId 群组ID
     * @param localUserId 本地用户ID
     * @param excludeUserId 要排除的用户ID（如消息原发送者）
     * @return 目标用户ID列表
     */
    Q_INVOKABLE QStringList getMessageTargets(const QString& groupId,
                                              const QString& localUserId,
                                              const QString& excludeUserId = QString()) const;

    /**
     * @brief 获取消息中继目标列表（群主专用）
     * 
     * 当群主收到成员消息时，需要转发给其他所有成员
     * 
     * @param groupId 群组ID
     * @param localUserId 本地用户ID（必须是群主）
     * @param fromUserId 消息原发送者
     * @param toUserId 消息原接收者
     * @return 需要中继的用户ID列表
     */
    Q_INVOKABLE QStringList getRelayTargets(const QString& groupId,
                                            const QString& localUserId,
                                            const QString& fromUserId,
                                            const QString& toUserId) const;

signals:
    /**
     * @brief 群组信息更新
     * @param groupId 群组ID
     */
    void groupUpdated(const QString& groupId);

    /**
     * @brief 成员加入群组
     * @param groupId 群组ID
     * @param userId 新成员ID
     */
    void memberJoined(const QString& groupId, const QString& userId);

    /**
     * @brief 成员离开群组
     * @param groupId 群组ID
     * @param userId 离开的成员ID
     */
    void memberLeft(const QString& groupId, const QString& userId);

    /**
     * @brief 群组被删除
     * @param groupId 群组ID
     */
    void groupRemoved(const QString& groupId);

private:
    explicit GroupChatManager(QObject* parent = nullptr);
    ~GroupChatManager() override = default;

    // 禁止拷贝
    GroupChatManager(const GroupChatManager&) = delete;
    GroupChatManager& operator=(const GroupChatManager&) = delete;

    /**
     * @brief 群组元数据结构
     */
    struct GroupMeta {
        QString ownerId;        ///< 群主ID
        QStringList members;    ///< 成员列表（不包含群主）
    };

    mutable QMutex m_mutex;                   ///< 线程安全锁
    QHash<QString, GroupMeta> m_groups;       ///< 群组元数据存储
    static GroupChatManager* s_instance;      ///< 单例实例
};

} // namespace services
} // namespace core
} // namespace flykylin

#endif // FLYKYLIN_CORE_SERVICES_GROUPCHATMANAGER_H
