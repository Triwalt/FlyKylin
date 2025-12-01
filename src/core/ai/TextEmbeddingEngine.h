#pragma once

#include <QString>
#include <vector>

namespace flykylin {
namespace ai {

/**
 * @brief Skeleton for text embedding engine used by semantic chat search.
 *
 * Current implementation is a stub and always reports unavailable. It is
 * designed so that future ONNX / RKNN based backends can be plugged in
 * transparently without changing callers.
 */
class TextEmbeddingEngine {
public:
    static TextEmbeddingEngine* instance();

    /**
     * @brief Whether a concrete embedding backend is available.
     */
    bool isAvailable() const;

    /**
     * @brief Compute embedding vector for the given text.
     *
     * When no backend is configured, this returns an empty vector.
     */
    std::vector<float> computeEmbedding(const QString& text) const;

    /**
     * @brief Dimension of the embedding vectors.
     */
    int embeddingDim() const;

    /**
     * @brief Human readable backend name (e.g. "ONNX-DirectML", "RKNN", "unavailable").
     */
    QString backendName() const;

private:
    TextEmbeddingEngine();
};

} // namespace ai
} // namespace flykylin
