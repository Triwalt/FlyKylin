#include "ChatSearchService.h"

#include "../database/DatabaseService.h"
#include "../ai/TextEmbeddingEngine.h"
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <vector>

namespace flykylin {
namespace services {

using database::DatabaseService;

QList<core::Message> ChatSearchService::search(const QString& localUserId,
                                               const QString& query,
                                               const SearchFilter& filter,
                                               bool useSemantic) const
{
    const QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    const QString peerId = filter.peerId;
    const int limit = filter.limit > 0 ? filter.limit : 200;
    const int candidateLimit = std::min(limit * 5, 1000);

    auto* db = DatabaseService::instance();
    if (!db) {
        qWarning() << "[ChatSearchService] DatabaseService instance is null";
        return {};
    }

    QList<core::Message> base =
        db->searchMessagesByKeyword(localUserId, trimmed, peerId, candidateLimit);

    const bool semanticAvailable =
        useSemantic && ai::TextEmbeddingEngine::instance()->isAvailable();

    if (!semanticAvailable || base.isEmpty()) {
        if (base.size() > limit) {
            base = base.mid(0, limit);
        }
        return base;
    }

    auto* engine = ai::TextEmbeddingEngine::instance();
    const std::vector<float> queryEmbedding = engine->computeEmbedding(trimmed);
    if (queryEmbedding.empty()) {
        if (base.size() > limit) {
            base = base.mid(0, limit);
        }
        return base;
    }

    struct ScoredMessage {
        core::Message message;
        float score;
    };

    std::vector<ScoredMessage> scored;
    scored.reserve(static_cast<std::size_t>(base.size()));

    for (const auto& msg : base) {
        const std::vector<float> embedding = engine->computeEmbedding(msg.content());
        if (embedding.size() != queryEmbedding.size() || embedding.empty()) {
            continue;
        }

        float dot = 0.0f;
        float normQuery = 0.0f;
        float normMsg = 0.0f;

        for (std::size_t i = 0; i < embedding.size(); ++i) {
            const float qv = queryEmbedding[i];
            const float mv = embedding[i];
            dot += qv * mv;
            normQuery += qv * qv;
            normMsg += mv * mv;
        }

        const float denom = std::sqrt(normQuery) * std::sqrt(normMsg);
        const float score = (denom > 0.0f) ? (dot / denom) : 0.0f;

        scored.push_back(ScoredMessage{msg, score});
    }

    if (scored.empty()) {
        if (base.size() > limit) {
            base = base.mid(0, limit);
        }
        return base;
    }

    std::sort(scored.begin(), scored.end(),
              [](const ScoredMessage& a, const ScoredMessage& b) {
                  if (a.score != b.score) {
                      return a.score > b.score;
                  }
                  return a.message.timestamp() > b.message.timestamp();
              });

    QList<core::Message> result;
    const int resultCount = std::min(limit, static_cast<int>(scored.size()));
    result.reserve(resultCount);
    for (int i = 0; i < resultCount; ++i) {
        result.append(scored[static_cast<std::size_t>(i)].message);
    }

    qInfo() << "[ChatSearchService] Semantic rerank used for query" << trimmed
            << "base=" << base.size() << "returned=" << result.size() << "peer="
            << peerId;

    return result;
}

} // namespace services
} // namespace flykylin
