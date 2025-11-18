/**
 * @file PeerListViewModel.cpp
 * @brief 在线用户列表的MVVM视图模型实现
 */

#include "PeerListViewModel.h"
#include <QDebug>

namespace flykylin {
namespace ui {

PeerListViewModel::PeerListViewModel(QObject* parent)
    : QObject(parent)
    , m_model(new QStandardItemModel(this))
{
    // 设置列标题
    m_model->setHorizontalHeaderLabels({"用户", "状态"});
    qDebug() << "[PeerListViewModel] Created";
}

void PeerListViewModel::setFilterKeyword(const QString& keyword)
{
    if (m_filterKeyword == keyword) {
        return;
    }
    
    m_filterKeyword = keyword;
    emit filterKeywordChanged();
    
    // 重新应用过滤
    updateModel();
}

void PeerListViewModel::onPeerDiscovered(const flykylin::core::PeerNode& peer)
{
    QString userId = peer.userId();
    
    qDebug() << "[PeerListViewModel] Peer discovered:" << userId
             << peer.userName() << "@" << peer.hostName();
    
    // 更新或添加节点
    m_peers[userId] = peer;
    
    // 更新显示
    updateModel();
    emit onlineCountChanged();
}

void PeerListViewModel::onPeerOffline(const QString& userId)
{
    qDebug() << "[PeerListViewModel] Peer offline:" << userId;
    
    if (m_peers.remove(userId) > 0) {
        updateModel();
        emit onlineCountChanged();
    }
}

void PeerListViewModel::onPeerHeartbeat(const QString& userId)
{
    if (m_peers.contains(userId)) {
        m_peers[userId].updateLastSeen();
        
        // 心跳不需要完全重建模型，只更新时间戳即可
        // 为了简化实现，暂时不单独更新
        qDebug() << "[PeerListViewModel] Heartbeat from:" << userId;
    }
}

void PeerListViewModel::updateModel()
{
    // 清空现有模型
    m_model->removeRows(0, m_model->rowCount());
    
    // 遍历所有节点，应用过滤并添加到模型
    for (auto it = m_peers.constBegin(); it != m_peers.constEnd(); ++it) {
        const auto& peer = it.value();
        
        if (!matchesFilter(peer)) {
            continue;
        }
        
        QList<QStandardItem*> rowItems;
        
        // 用户名列
        auto* nameItem = createPeerItem(peer);
        rowItems.append(nameItem);
        
        // 状态列
        auto* statusItem = new QStandardItem();
        statusItem->setText(peer.isOnline() ? "在线" : "离线");
        statusItem->setForeground(peer.isOnline() ? QColor(Qt::green) : QColor(Qt::gray));
        statusItem->setEditable(false);
        rowItems.append(statusItem);
        
        m_model->appendRow(rowItems);
    }
    
    qDebug() << "[PeerListViewModel] Model updated, total:" << m_model->rowCount();
}

QStandardItem* PeerListViewModel::createPeerItem(const flykylin::core::PeerNode& peer)
{
    auto* item = new QStandardItem();
    
    // 显示文本：用户名 (主机名)
    QString displayText = QString("%1 (%2)")
                            .arg(peer.userName().isEmpty() ? peer.userId() : peer.userName())
                            .arg(peer.hostName());
    item->setText(displayText);
    
    // 设置工具提示显示详细信息
    QString tooltip = QString("用户ID: %1\n主机: %2\nIP: %3\nTCP端口: %4\n最后心跳: %5")
                        .arg(peer.userId())
                        .arg(peer.hostName())
                        .arg(peer.ipAddress().toString())
                        .arg(peer.tcpPort())
                        .arg(peer.lastSeen().toString("yyyy-MM-dd hh:mm:ss"));
    item->setToolTip(tooltip);
    
    // 存储完整的userId用于后续操作
    item->setData(peer.userId(), Qt::UserRole);
    
    // 设置为不可编辑
    item->setEditable(false);
    
    return item;
}

bool PeerListViewModel::matchesFilter(const flykylin::core::PeerNode& peer) const
{
    if (m_filterKeyword.isEmpty()) {
        return true;
    }
    
    // 搜索用户名、主机名、IP
    QString keyword = m_filterKeyword.toLower();
    
    return peer.userName().toLower().contains(keyword) ||
           peer.hostName().toLower().contains(keyword) ||
           peer.userId().toLower().contains(keyword) ||
           peer.ipAddress().toString().contains(keyword);
}

} // namespace ui
} // namespace flykylin
