/**
 * @file PeerListViewModel.h
 * @brief 在线用户列表的MVVM视图模型
 */

#ifndef PEERLISTVIEWMODEL_H
#define PEERLISTVIEWMODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <QMap>
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

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit PeerListViewModel(QObject* parent = nullptr);

    /**
     * @brief 获取数据模型（用于QListView绑定）
     * @return 标准item模型指针
     */
    QStandardItemModel* peerListModel() const { return m_model; }

    /**
     * @brief 获取在线用户数量
     * @return 在线用户数
     */
    int onlineCount() const { return m_peers.count(); }

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
    QStandardItemModel* m_model;                        ///< 数据模型
    QMap<QString, flykylin::core::PeerNode> m_peers;    ///< userId -> PeerNode
    QString m_filterKeyword;                            ///< 过滤关键词
};

} // namespace ui
} // namespace flykylin

#endif // PEERLISTVIEWMODEL_H
