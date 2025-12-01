/**
 * @file PeerListViewModel.h
 * @brief 在线用户列表的MVVM视图模型
 */

#ifndef PEERLISTVIEWMODEL_H
#define PEERLISTVIEWMODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <QMap>
#include <QStringList>
#include "../../core/models/PeerNode.h"

namespace flykylin {
namespace ui {

/**
 * @class PeerListViewModel
 * @brief 用户列表的视图模型，遵循MVVM模式
 * 
 * 负责：
 * - 维护在线用户数据模型
 * - 处理搜索过滤
 * - 与PeerDiscovery集成
 */
class PeerListViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(int onlineCount READ onlineCount NOTIFY onlineCountChanged)
    Q_PROPERTY(QString filterKeyword READ filterKeyword WRITE setFilterKeyword NOTIFY filterKeywordChanged)
    Q_PROPERTY(QStandardItemModel* peerListModel READ peerListModel CONSTANT)
    Q_PROPERTY(QStandardItemModel* sessionListModel READ sessionListModel CONSTANT)
    Q_PROPERTY(int sessionCount READ sessionCount NOTIFY sessionCountChanged)
    Q_PROPERTY(QString currentPeerId READ currentPeerId NOTIFY currentPeerIdChanged)
    Q_PROPERTY(int onlineVersion READ onlineVersion NOTIFY onlineVersionChanged)

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit PeerListViewModel(QObject* parent = nullptr);

    /**
     * @brief 获取当前选中的用户ID
     */
    QString currentPeerId() const { return m_currentPeerId; }

    /**
     * @brief 获取数据模型（用于QListView绑定）
     * @return 标准item模型指针
     */
    QStandardItemModel* peerListModel() const { return m_model; }
    QStandardItemModel* sessionListModel() const { return m_sessionModel; }

    /**
     * @brief 获取在线用户数量
     * @return 在线用户数
     */
    int onlineCount() const
    {
        int count = 0;
        for (auto it = m_peers.constBegin(); it != m_peers.constEnd(); ++it) {
            if (it.value().isOnline()) {
                ++count;
            }
        }
        return count;
    }

    int sessionCount() const { return m_sessionModel ? m_sessionModel->rowCount() : 0; }

    int onlineVersion() const { return m_onlineVersion; }

    /**
     * @brief 获取过滤关键词
     * @return 当前过滤关键词
     */
    QString filterKeyword() const { return m_filterKeyword; }

    /**
     * @brief 设置过滤关键词
     * @param keyword 关键词
     */
    void setFilterKeyword(const QString& keyword);

    /**
     * @brief 选中用户
     * @param userId 用户ID
     */
    Q_INVOKABLE void selectPeer(const QString& userId);

    Q_INVOKABLE void requestPeerDetails(const QString& userId);
    Q_INVOKABLE void requestAddToContacts(const QString& userId);
    Q_INVOKABLE void requestAddToGroup(const QString& userId);
    Q_INVOKABLE void deleteSession(const QString& userId);

    /**
     * @brief 查询指定用户当前是否在线
     */
    Q_INVOKABLE bool isPeerOnline(const QString& userId) const;

    // 从联系人数据补齐/更新指定用户的显示信息（用户名、主机名、IP、端口等）
    Q_INVOKABLE void upsertPeerFromContact(const QString& userId,
                                           const QString& userName,
                                           const QString& hostName,
                                           const QString& ipAddress,
                                           quint16 tcpPort);

    // 从数据库加载历史会话（包含已经离线但有聊天记录的用户）
    void loadHistoricalSessions();

public slots:
    /**
     * @brief 处理发现新节点事件
     * @param peer 节点信息
     */
    void onPeerDiscovered(const flykylin::core::PeerNode& peer);

    /**
     * @brief 处理节点离线事件
     * @param userId 用户ID
     */
    void onPeerOffline(const QString& userId);

    /**
     * @brief 处理节点心跳事件
     * @param userId 用户ID
     */
    void onPeerHeartbeat(const QString& userId);

signals:
    /**
     * @brief 在线用户数量变化信号
     */
    void onlineCountChanged();

    /**
     * @brief 过滤关键词变化信号
     */
    void filterKeywordChanged();

    /**
     * @brief 用户被选中信号
     * @param peer 被选中的节点
     */
    void peerSelected(const flykylin::core::PeerNode& peer);

    /**
     * @brief 当前选中用户ID变化信号
     */
    void currentPeerIdChanged();

    void onlineVersionChanged();
    void sessionCountChanged();

    void peerDetailsRequested(const QString& userId,
                              const QString& userName,
                              const QString& hostName,
                              const QString& ipAddress,
                              quint16 tcpPort,
                              const QStringList& groups);

    void addToContactsRequested(const QString& userId,
                                const QString& userName,
                                const QString& hostName,
                                const QString& ipAddress,
                                quint16 tcpPort);

    void addToGroupRequested(const QString& userId,
                             const QString& userName,
                             const QString& hostName,
                             const QString& ipAddress,
                             quint16 tcpPort);

    void peerOfflineNotified(const QString& userId,
                             const QString& userName,
                             const QString& ipAddress);

private:
    /**
     * @brief 更新显示模型（应用过滤）
     */
    void updateModel();

    /**
     * @brief 创建节点对应的Item
     * @param peer 节点信息
     * @return QStandardItem指针
     */
    QStandardItem* createPeerItem(const flykylin::core::PeerNode& peer);

    /**
     * @brief 检查节点是否匹配过滤条件
     * @param peer 节点信息
     * @return 匹配返回true
     */
    bool matchesFilter(const flykylin::core::PeerNode& peer) const;

private:
    QStandardItemModel* m_model;                        ///< 在线用户数据模型
    QStandardItemModel* m_sessionModel;                 ///< 会话列表数据模型（包含离线用户）
    QMap<QString, flykylin::core::PeerNode> m_peers;    ///< userId -> PeerNode
    QString m_filterKeyword;                            ///< 过滤关键词
    QString m_currentPeerId;                            ///< 当前选中用户ID
    int m_onlineVersion = 0;                            ///< 在线状态版本号（用于QML绑定刷新）
};

} // namespace ui
} // namespace flykylin

#endif // PEERLISTVIEWMODEL_H
