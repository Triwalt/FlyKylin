/**
 * @file RetryStrategy.h
 * @brief Smart retry strategy with exponential backoff and jitter
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#pragma once

#include <QString>

namespace flykylin {
namespace communication {

/**
 * @brief Retry strategy using exponential backoff with random jitter
 * 
 * Implements intelligent retry mechanism to avoid network congestion:
 * - Retry delays: 1s, 2s, 4s, 8s, 16s (max 5 retries)
 * - Random jitter: ±20% to prevent synchronized retries
 * - Max delay: 30 seconds
 */
class RetryStrategy {
public:
    /**
     * @brief Get next retry delay in milliseconds
     * @param retryCount Current retry count (0-based)
     * @return Delay in milliseconds, -1 if should give up
     */
    static int getNextRetryDelay(int retryCount);
    
    /**
     * @brief Get user-friendly retry message
     * @param retryCount Current retry count
     * @return Localized retry message for GUI display
     */
    static QString getRetryMessage(int retryCount);
    
    /**
     * @brief Check if should give up retrying
     * @param retryCount Current retry count
     * @return true if should give up
     */
    static bool shouldGiveUp(int retryCount);
    
private:
    static constexpr int kMaxRetryCount = 5;      ///< Max 5 retries
    static constexpr int kBaseDelay = 1000;       ///< Base delay 1 second
    static constexpr int kMaxDelay = 30000;       ///< Max delay 30 seconds
    static constexpr int kJitterPercent = 20;     ///< Jitter ±20%
};

} // namespace communication
} // namespace flykylin
