#include "ChatSearchService.h"

#include "../ai/TextEmbeddingEngine.h"
#include "../database/DatabaseService.h"
#include <QDebug>
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>
#include <vector>

namespace flykylin {
namespace services {

using database::DatabaseService;

namespace {
// 语义重排的最大候选数量 - 限制RKNN推理次数以保证响应速度
// 在RK3566上，每次BGE推理约需100-200ms，10条约需1-2秒
constexpr int kMaxSemanticCandidates = 10;

// 计算余弦相似度
float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b)
{
    if (a.size() != b.size() || a.empty()) {
        return 0.0f;
    }

    float dot = 0.0f;
    float normA = 0.0f;
    float normB = 0.0f;

    for (std::size_t i = 0; i < a.size(); ++i) {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }

    const float denom = std::sqrt(normA) * std::sqrt(normB);
    return (denom > 0.0f) ? (dot / denom) : 0.0f;
}
} // namespace

QList<core::Message> ChatSearchService::search(const QString& localUserId,
                                               const QString& query,
                                               const SearchFilter& filter,
                                               bool useSemantic) const
{
    QElapsedTimer timer;
    timer.start();

    const QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    const QString peerId = filter.peerId;
    const int limit = filter.limit > 0 ? filter.limit : 50;

    auto* db = DatabaseService::instance();
    if (!db) {
        qWarning() << "[ChatSearchService] DatabaseService instance is null";
        return {};
    }

    const bool engineAvailable = ai::TextEmbeddingEngine::instance()->isAvailable();
    const bool semanticAvailable = useSemantic && engineAvailable;

    // 优化策略：
    // 1. 始终先用关键字过滤，减少候选数量
    // 2. 语义搜索只对关键字匹配的前N条进行重排
    // 3. 如果关键字匹配结果太少，再扩大搜索范围

    QList<core::Message> keywordMatches =
        db->searchMessagesByKeyword(localUserId, trimmed, peerId, limit * 2);

    qInfo() << "[ChatSearchService] Keyword search for" << trimmed << "found"
            << keywordMatches.size() << "matches in" << timer.elapsed() << "ms";

    // 如果不使用语义搜索，或者引擎不可用，直接返回关键字结果
    if (!semanticAvailable) {
        if (keywordMatches.size() > limit) {
            keywordMatches = keywordMatches.mid(0, limit);
        }
        qInfo() << "[ChatSearchService] Returning keyword results:" << keywordMatches.size()
                << "total time:" << timer.elapsed() << "ms";
        return keywordMatches;
    }

    // 语义搜索：对关键字匹配结果进行语义重排
    auto* engine = ai::TextEmbeddingEngine::instance();

    // 计算查询的embedding
    QElapsedTimer embedTimer;
    embedTimer.start();
    const std::vector<float> queryEmbedding = engine->computeEmbedding(trimmed);
    qInfo() << "[ChatSearchService] Query embedding computed in" << embedTimer.elapsed() << "ms";

    if (queryEmbedding.empty()) {
        // embedding计算失败，回退到关键字结果
        if (keywordMatches.size() > limit) {
            keywordMatches = keywordMatches.mid(0, limit);
        }
        qInfo() << "[ChatSearchService] Embedding failed, returning keyword results:"
                << keywordMatches.size();
        return keywordMatches;
    }

    // 限制语义重排的候选数量，避免过长的处理时间
    const int semanticCandidateCount =
        std::min(static_cast<int>(keywordMatches.size()), kMaxSemanticCandidates);

    if (semanticCandidateCount == 0) {
        qInfo() << "[ChatSearchService] No candidates for semantic ranking";
        return keywordMatches;
    }

    qInfo() << "[ChatSearchService] Computing embeddings for" << semanticCandidateCount
            << "candidates...";

    struct ScoredMessage {
        core::Message message;
        float score;
    };

    std::vector<ScoredMessage> scored;
    scored.reserve(static_cast<std::size_t>(semanticCandidateCount));

    embedTimer.restart();
    for (int i = 0; i < semanticCandidateCount; ++i) {
        const auto& msg = keywordMatches.at(i);
        const std::vector<float> msgEmbedding = engine->computeEmbedding(msg.content());

        if (msgEmbedding.empty()) {
            // embedding失败，给一个较低的默认分数
            scored.push_back(ScoredMessage{msg, 0.0f});
            continue;
        }

        const float score = cosineSimilarity(queryEmbedding, msgEmbedding);
        scored.push_back(ScoredMessage{msg, score});
    }

    qInfo() << "[ChatSearchService] Computed" << scored.size() << "embeddings in"
            << embedTimer.elapsed() << "ms";

    // 按相似度排序
    std::sort(scored.begin(), scored.end(), [](const ScoredMessage& a, const ScoredMessage& b) {
        if (std::abs(a.score - b.score) > 0.001f) {
            return a.score > b.score;
        }
        // 相似度相近时，按时间降序
        return a.message.timestamp() > b.message.timestamp();
    });

    // 构建结果：语义排序的结果 + 剩余的关键字匹配结果
    QList<core::Message> result;
    result.reserve(limit);

    // 添加语义排序的结果
    for (const auto& sm : scored) {
        if (result.size() >= limit) {
            break;
        }
        result.append(sm.message);
    }

    // 如果结果不够，添加剩余的关键字匹配结果
    for (int i = semanticCandidateCount; i < keywordMatches.size() && result.size() < limit; ++i) {
        result.append(keywordMatches.at(i));
    }

    qInfo() << "[ChatSearchService] Semantic search complete: query=" << trimmed
            << "keyword_matches=" << keywordMatches.size()
            << "semantic_ranked=" << scored.size() << "returned=" << result.size()
            << "total_time=" << timer.elapsed() << "ms";

    return result;
}

} // namespace services
} // namespace flykylin
