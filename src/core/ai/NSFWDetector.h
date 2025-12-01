#pragma once

#include <QString>
#include <optional>

namespace flykylin {
namespace ai {

class NSFWDetector {
public:
    static NSFWDetector* instance();

    bool isAvailable() const;

    std::optional<float> predictNsfwProbability(const QString& imagePath) const;

private:
    NSFWDetector();
};

} // namespace ai
} // namespace flykylin
