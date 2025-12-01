#include "GlobalSearchViewModel.h"

#include "../../core/config/UserProfile.h"
#include <QDebug>

namespace flykylin {
namespace ui {

GlobalSearchViewModel::GlobalSearchViewModel(QObject* parent)
    : QObject(parent)
    , m_resultsModel(new QStandardItemModel(this))
{
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole] = "content";
    roles[Qt::UserRole + 1] = "peerId";
    roles[Qt::UserRole + 2] = "timestamp";
    roles[Qt::UserRole + 3] = "messageId";
    m_resultsModel->setItemRoleNames(roles);
}

GlobalSearchViewModel::~GlobalSearchViewModel()
{
    qInfo() << "[GlobalSearchViewModel] Destroyed";
}

void GlobalSearchViewModel::clearResults()
{
    if (m_resultsModel) {
        m_resultsModel->clear();
    }
}

void GlobalSearchViewModel::search(const QString& query,
                                   bool useSemantic,
                                   const QString& peerId)
{
    const QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) {
        clearResults();
        return;
    }

    const QString localUserId = core::UserProfile::instance().userId();

    services::SearchFilter filter;
    filter.peerId = peerId;
    filter.limit = 200;

    const QList<core::Message> messages =
        m_searchService.search(localUserId, trimmed, filter, useSemantic);

    clearResults();

    for (const auto& msg : messages) {
        const QString resolvedPeerId =
            (msg.fromUserId() == localUserId) ? msg.toUserId() : msg.fromUserId();

        auto* item = new QStandardItem();
        item->setData(msg.content(), Qt::UserRole);
        item->setData(resolvedPeerId, Qt::UserRole + 1);
        item->setData(msg.timestamp().toString("yyyy-MM-dd HH:mm:ss"), Qt::UserRole + 2);
        item->setData(msg.id(), Qt::UserRole + 3);
        m_resultsModel->appendRow(item);
    }

    qInfo() << "[GlobalSearchViewModel] Search query=" << trimmed
            << "results=" << m_resultsModel->rowCount()
            << "peerFilter=" << peerId;
}

} // namespace ui
} // namespace flykylin
