/**
 * @file RetryStrategy.cpp
 * @brief Implementation of retry strategy with exponential backoff
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#include "RetryStrategy.h"
#include <QRandomGenerator>
#include <algorithm>

namespace flykylin {
namespace communication {

int RetryStrategy::getNextRetryDelay(int retryCount) {
    if (shouldGiveUp(retryCount)) {
        return -1;  // Give up
    }
    
    // Exponential backoff: 1s, 2s, 4s, 8s, 16s
    int baseDelay = kBaseDelay * (1 << retryCount);
    
    // Add random jitter (±20%) to prevent synchronized retries
    int jitterRange = baseDelay * kJitterPercent / 100;
    int jitter = QRandomGenerator::global()->bounded(-jitterRange, jitterRange);
    
    // Cap at maximum delay
    return std::min(baseDelay + jitter, kMaxDelay);
}

QString RetryStrategy::getRetryMessage(int retryCount) {
    if (retryCount == 0) {
        return QStringLiteral("Connecting...");
    }
    return QStringLiteral("Connection failed, retrying (attempt %1)...").arg(retryCount);
}

bool RetryStrategy::shouldGiveUp(int retryCount) {
    return retryCount >= kMaxRetryCount;
}

} // namespace communication
} // namespace flykylin
