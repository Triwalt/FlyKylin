/**
 * @file PeerListViewModel.cpp
 * @brief 在线用户列表的MVVM视图模型实现
 */

#include "PeerListViewModel.h"
#include <QDebug>
#include "../../core/config/UserProfile.h"
#include "../../core/database/DatabaseService.h"

namespace flykylin {
namespace ui {

PeerListViewModel::PeerListViewModel(QObject* parent)
    : QObject(parent)
    , m_model(new QStandardItemModel(this))
    , m_sessionModel(new QStandardItemModel(this))
{
    // 设置列标题
    m_model->setHorizontalHeaderLabels({"用户", "状态"});
    m_sessionModel->setHorizontalHeaderLabels({"用户", "状态"});

    // Set role names for QML
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display"; // Default display text
    roles[Qt::UserRole] = "userId";
    roles[Qt::UserRole + 1] = "userName";
    roles[Qt::UserRole + 2] = "ipAddress";
    roles[Qt::UserRole + 3] = "isOnline";
    roles[Qt::UserRole + 4] = "tcpPort";
    m_model->setItemRoleNames(roles);
    m_sessionModel->setItemRoleNames(roles);

    qDebug() << "[PeerListViewModel] Created";
}

void PeerListViewModel::upsertPeerFromContact(const QString& userId,
                                              const QString& userName,
                                              const QString& hostName,
                                              const QString& ipAddress,
                                              quint16 tcpPort)
{
    if (userId.isEmpty()) {
        return;
    }

    flykylin::core::PeerNode node;
    auto it = m_peers.find(userId);
    if (it != m_peers.end()) {
        node = it.value();
    } else {
        node.setUserId(userId);
        node.setOnline(false);
    }

    if (!userName.isEmpty()) {
        node.setUserName(userName);
    }
    if (!hostName.isEmpty()) {
        node.setHostName(hostName);
    }
    if (!ipAddress.isEmpty()) {
        node.setIpAddress(ipAddress);
    }
    if (tcpPort > 0) {
        node.setTcpPort(tcpPort);
    }

    m_peers.insert(userId, node);

    updateModel();
}

void PeerListViewModel::loadHistoricalSessions()
{
    const QString localUserId = flykylin::core::UserProfile::instance().userId();
    auto* db = flykylin::database::DatabaseService::instance();
    const auto sessions = db->loadSessions(localUserId);

    qDebug() << "[PeerListViewModel] Loading historical sessions for" << localUserId
             << "count=" << sessions.size();

    for (const auto& entry : sessions) {
        const QString& peerId = entry.first;
        const qint64 lastTs = entry.second;

        if (m_peers.contains(peerId)) {
            continue; // 已经由PeerDiscovery填充
        }

        flykylin::core::PeerNode node;
        node.setUserId(peerId);
        node.setUserName(peerId); // 缺省情况下使用ID作为名称
        node.setHostName(QString());
        node.setIpAddress(QString());
        node.setTcpPort(0);
        node.setOnline(false);
        node.setLastSeen(QDateTime::fromMSecsSinceEpoch(lastTs));

        m_peers.insert(peerId, node);
    }

    updateModel();
    emit sessionCountChanged();
}

void PeerListViewModel::selectPeer(const QString& userId)
{
    if (m_peers.contains(userId)) {
        qDebug() << "[PeerListViewModel] Selected peer:" << userId;

        // 始终通知 ChatViewModel 打开该会话，即使当前选中ID未变
        if (m_currentPeerId != userId) {
            m_currentPeerId = userId;
            emit currentPeerIdChanged();
        }

        emit peerSelected(m_peers[userId]);
    } else {
        qWarning() << "[PeerListViewModel] Selected peer not found:" << userId;
    }
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

bool PeerListViewModel::isPeerOnline(const QString& userId) const
{
    auto it = m_peers.constFind(userId);
    if (it == m_peers.constEnd()) {
        return false;
    }
    return it->isOnline();
}

void PeerListViewModel::requestPeerDetails(const QString& userId)
{
    if (!m_peers.contains(userId)) {
        qWarning() << "[PeerListViewModel] requestPeerDetails: peer not found" << userId;
        return;
    }

    const auto& peer = m_peers[userId];
    QStringList groups;
    emit peerDetailsRequested(peer.userId(),
                              peer.userName(),
                              peer.hostName(),
                              peer.ipAddress().toString(),
                              peer.tcpPort(),
                              groups);
}

void PeerListViewModel::requestAddToContacts(const QString& userId)
{
    if (!m_peers.contains(userId)) {
        qWarning() << "[PeerListViewModel] requestAddToContacts: peer not found" << userId;
        return;
    }

    const auto& peer = m_peers[userId];
    emit addToContactsRequested(peer.userId(),
                                peer.userName(),
                                peer.hostName(),
                                peer.ipAddress().toString(),
                                peer.tcpPort());
}

void PeerListViewModel::requestAddToGroup(const QString& userId)
{
    if (!m_peers.contains(userId)) {
        qWarning() << "[PeerListViewModel] requestAddToGroup: peer not found" << userId;
        return;
    }

    const auto& peer = m_peers[userId];
    emit addToGroupRequested(peer.userId(),
                             peer.userName(),
                             peer.hostName(),
                             peer.ipAddress().toString(),
                             peer.tcpPort());
}

void PeerListViewModel::onPeerDiscovered(const flykylin::core::PeerNode& peer)
{
    QString userId = peer.userId();
    
    qDebug() << "[PeerListViewModel] Peer discovered:" << userId
             << peer.userName() << "@" << peer.hostName();
    
    // 更新或添加节点
    bool isNewPeer = !m_peers.contains(userId);
    flykylin::core::PeerNode updatedPeer = peer;
    updatedPeer.setOnline(true);
    m_peers[userId] = updatedPeer;

    // 更新显示
    updateModel();
    emit onlineCountChanged();
    ++m_onlineVersion;
    emit onlineVersionChanged();

    // 如果当前选中的节点被更新（例如对方修改了昵称），重新发出选中信号，
    // 让 ChatViewModel 等订阅者刷新显示名称。
    if (!isNewPeer && m_currentPeerId == userId) {
        emit peerSelected(peer);
    }
}

void PeerListViewModel::onPeerOffline(const QString& userId)
{
    qDebug() << "[PeerListViewModel] Peer offline:" << userId;

    auto it = m_peers.find(userId);
    if (it == m_peers.end()) {
        return;
    }

    flykylin::core::PeerNode peer = it.value();
    peer.setOnline(false);
    it.value() = peer;
    emit peerOfflineNotified(peer.userId(),
                             peer.userName(),
                             peer.ipAddress().toString());

    updateModel();
    emit onlineCountChanged();
    ++m_onlineVersion;
    emit onlineVersionChanged();
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
    m_sessionModel->removeRows(0, m_sessionModel->rowCount());
    
    // 遍历所有节点，应用过滤并添加到模型
    for (auto it = m_peers.constBegin(); it != m_peers.constEnd(); ++it) {
        const auto& peer = it.value();
        
        if (!matchesFilter(peer)) {
            continue;
        }
        
        // 会话列表：包含在线和离线用户
        QList<QStandardItem*> sessionRow;

        auto* sessionNameItem = createPeerItem(peer);
        sessionRow.append(sessionNameItem);

        auto* sessionStatusItem = new QStandardItem();
        sessionStatusItem->setText(peer.isOnline() ? "在线" : "离线");
        sessionStatusItem->setForeground(peer.isOnline() ? QColor(Qt::green) : QColor(Qt::gray));
        sessionStatusItem->setEditable(false);
        sessionRow.append(sessionStatusItem);

        m_sessionModel->appendRow(sessionRow);

        // 在线列表：仅包含当前在线用户
        if (peer.isOnline()) {
            QList<QStandardItem*> onlineRow;

            auto* onlineNameItem = createPeerItem(peer);
            onlineRow.append(onlineNameItem);

            auto* onlineStatusItem = new QStandardItem();
            onlineStatusItem->setText("在线");
            onlineStatusItem->setForeground(QColor(Qt::green));
            onlineStatusItem->setEditable(false);
            onlineRow.append(onlineStatusItem);

            m_model->appendRow(onlineRow);
        }
    }
    
    qDebug() << "[PeerListViewModel] Model updated, online:" << m_model->rowCount()
             << "sessions:" << m_sessionModel->rowCount();
    emit sessionCountChanged();
}

QStandardItem* PeerListViewModel::createPeerItem(const flykylin::core::PeerNode& peer)
{
    auto* item = new QStandardItem();
    
    // 显示文本：用户名 (主机名) [IP:端口]
    QString namePart = peer.userName().isEmpty() ? peer.userId() : peer.userName();
    QString hostPart = peer.hostName().isEmpty() ? "-" : peer.hostName();
    QString ipPart = peer.ipAddress().toString();
    QString portPart = peer.tcpPort() > 0 ? QString::number(peer.tcpPort()) : QStringLiteral("-");

    QString displayText = QString("%1 (%2) [%3:%4]")
                            .arg(namePart)
                            .arg(hostPart)
                            .arg(ipPart)
                            .arg(portPart);
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
    item->setData(peer.userName(), Qt::UserRole + 1);
    item->setData(peer.ipAddress().toString(), Qt::UserRole + 2);
    item->setData(peer.isOnline(), Qt::UserRole + 3);
    item->setData(static_cast<int>(peer.tcpPort()), Qt::UserRole + 4);
    
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
