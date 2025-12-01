#pragma once

#include <QObject>
#include <QStandardItemModel>
#include <QString>

#include "../../core/services/ChatSearchService.h"
#include "../../core/models/Message.h"

namespace flykylin {
namespace ui {

/**
 * @brief View model exposing global chat search results to QML.
 *
 * Currently this only performs keyword-based search via ChatSearchService.
 * The interface is designed so that semantic (embedding-based) search can be
 * enabled later without changing QML callers.
 */
class GlobalSearchViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStandardItemModel* resultsModel READ resultsModel CONSTANT)

public:
    explicit GlobalSearchViewModel(QObject* parent = nullptr);
    ~GlobalSearchViewModel() override;

    QStandardItemModel* resultsModel() const { return m_resultsModel; }

    Q_INVOKABLE void clearResults();

    /**
     * @brief Run a search over chat history.
     * @param query Search text entered by user.
     * @param useSemantic Whether to prefer semantic search (currently ignored,
     *                    reserved for future TextEmbeddingEngine integration).
     * @param peerId Optional peer filter; empty means search across all peers.
     */
    Q_INVOKABLE void search(const QString& query,
                            bool useSemantic,
                            const QString& peerId = QString());

private:
    QStandardItemModel* m_resultsModel;
    services::ChatSearchService m_searchService;
};

} // namespace ui
} // namespace flykylin
