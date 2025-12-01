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

    const bool engineAvailable = ai::TextEmbeddingEngine::instance()->isAvailable();
    const bool semanticAvailable = useSemantic && engineAvailable;

    QList<core::Message> base;
    if (semanticAvailable) {
        // 相关性排序：不再依赖 SQL 的 LIKE 预过滤，而是从所有消息中取候选再做语义重排。
        base = db->loadMessagesForSearch(localUserId, peerId, candidateLimit);
    } else {
        // 时间排序：仍然使用基于关键字的 SQL 过滤和按时间降序排序。
        base = db->searchMessagesByKeyword(localUserId, trimmed, peerId, candidateLimit);
    }

    if (!semanticAvailable || base.isEmpty()) {
        if (base.size() > limit) {
            base = base.mid(0, limit);
        }

        qInfo() << "[ChatSearchService] Time-based order for query" << trimmed
                << "base=" << base.size() << "peer=" << peerId;
        for (int i = 0; i < base.size(); ++i) {
            const auto& msg = base.at(i);
            qInfo() << "  [TimeSort]" << i
                    << msg.timestamp().toString("yyyy-MM-dd HH:mm:ss")
                    << msg.id()
                    << msg.content().left(32);
        }

        return base;
    }

    auto* engine = ai::TextEmbeddingEngine::instance();
    const std::vector<float> queryEmbedding = engine->computeEmbedding(trimmed);
    if (queryEmbedding.empty()) {
        if (base.size() > limit) {
            base = base.mid(0, limit);
        }

        qInfo() << "[ChatSearchService] Time-based order for query (no embedding)" << trimmed
                << "base=" << base.size() << "peer=" << peerId;
        for (int i = 0; i < base.size(); ++i) {
            const auto& msg = base.at(i);
            qInfo() << "  [TimeSort]" << i
                    << msg.timestamp().toString("yyyy-MM-dd HH:mm:ss")
                    << msg.id()
                    << msg.content().left(32);
        }

        return base;
    }

    qInfo() << "[ChatSearchService] Base (time-ordered) candidates for query" << trimmed
            << "base=" << base.size() << "peer=" << peerId;
    for (int i = 0; i < base.size(); ++i) {
        const auto& msg = base.at(i);
        qInfo() << "  [Base]" << i
                << msg.timestamp().toString("yyyy-MM-dd HH:mm:ss")
                << msg.id()
                << msg.content().left(32);
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

        qInfo() << "[ChatSearchService] Time-based order for query (no valid embeddings)"
                << trimmed << "base=" << base.size() << "peer=" << peerId;
        for (int i = 0; i < base.size(); ++i) {
            const auto& msg = base.at(i);
            qInfo() << "  [TimeSort]" << i
                    << msg.timestamp().toString("yyyy-MM-dd HH:mm:ss")
                    << msg.id()
                    << msg.content().left(32);
        }

        return base;
    }

    std::sort(scored.begin(), scored.end(),
              [](const ScoredMessage& a, const ScoredMessage& b) {
                  if (a.score != b.score) {
                      return a.score > b.score;
                  }
                  // 当语义得分相同时，使用时间升序作为 tie-breaker，
                  // 以便与按时间（降序）排序的结果产生明显差异。
                  return a.message.timestamp() < b.message.timestamp();
              });

    const int resultCount = std::min(limit, static_cast<int>(scored.size()));

    qInfo() << "[ChatSearchService] Semantic order for query" << trimmed
            << "base=" << base.size() << "returned=" << resultCount << "peer="
            << peerId;
    for (int i = 0; i < resultCount; ++i) {
        const auto& sm = scored[static_cast<std::size_t>(i)];
        qInfo() << "  [Semantic]" << i
                << sm.score
                << sm.message.timestamp().toString("yyyy-MM-dd HH:mm:ss")
                << sm.message.id()
                << sm.message.content().left(32);
    }

    QList<core::Message> result;
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
