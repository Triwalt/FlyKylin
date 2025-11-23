#pragma once

#include <string>
#include <vector>
#include <future>
#include <optional>

namespace flykylin::core::interfaces {

/**
 * @brief AI Accelerator Interface
 * 
 * Abstracts the underlying AI inference engine (ONNX Runtime DirectML vs RKNN).
 */
class I_Accelerator {
public:
    virtual ~I_Accelerator() = default;

    /**
     * @brief Initialize the accelerator
     * @return true if initialization successful
     */
    virtual bool initialize() = 0;

    /**
     * @brief Run inference on an image (e.g., for NSFW detection)
     * @param imageData Raw image data
     * @param width Image width
     * @param height Image height
     * @return Probability score (0.0 - 1.0) or error
     */
    virtual std::future<float> runInferenceAsync(const std::vector<uint8_t>& imageData, int width, int height) = 0;

    /**
     * @brief Get the name of the accelerator (e.g., "DirectML", "RKNN", "CPU")
     */
    virtual std::string getAcceleratorName() const = 0;
};

} // namespace flykylin::core::interfaces
