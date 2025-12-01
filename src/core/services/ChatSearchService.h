#pragma once

#include <QList>
#include <QString>
#include "core/models/Message.h"

namespace flykylin {
namespace services {

struct SearchFilter {
    QString peerId;   ///< Optional peer id filter; empty means search all peers
    int limit{200};   ///< Max number of messages to return
};

/**
 * @brief Chat search service entry point.
 *
 * Currently only performs keyword-based search via DatabaseService. The
 * interface is pre-designed so that a future semantic search path using
 * TextEmbeddingEngine can be added without changing callers.
 */
class ChatSearchService {
public:
    ChatSearchService() = default;

    QList<core::Message> search(const QString& localUserId,
                                const QString& query,
                                const SearchFilter& filter,
                                bool useSemantic) const;
};

} // namespace services
} // namespace flykylin
