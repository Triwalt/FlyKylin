/**
 * @file NetworkInterfaceCache.h
 * @brief Network interface caching for performance optimization
 * @author FlyKylin Development Team
 * @date 2024-11-20
 * 
 * Caches network interface information to avoid expensive QNetworkInterface::allInterfaces()
 * calls. Automatically refreshes the cache periodically.
 * 
 * @see TD-002 Network Interface Caching
 */

#pragma once

#include <QObject>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QTimer>
#include <QList>
#include <QMutex>

namespace flykylin {
namespace communication {

/**
 * @class NetworkInterfaceCache
 * @brief Caches network interface information with automatic refresh
 * 
 * Provides fast access to local network addresses by caching
 * QNetworkInterface::allInterfaces() results and refreshing periodically.
 * Thread-safe for concurrent access.
 * 
 * Usage:
 * @code
 * NetworkInterfaceCache cache;
 * cache.start();  // Start auto-refresh
 * 
 * // Fast lookup
 * bool isLocal = cache.isLocalAddress(address);
 * QList<QHostAddress> locals = cache.localAddresses();
 * @endcode
 */
class NetworkInterfaceCache : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     * @param refreshIntervalMs Cache refresh interval in milliseconds (default: 30000ms = 30s)
     */
    explicit NetworkInterfaceCache(QObject* parent = nullptr, int refreshIntervalMs = 30000);
    
    /**
     * @brief Destructor
     */
    ~NetworkInterfaceCache() override;
    
    /**
     * @brief Start automatic cache refresh
     */
    void start();
    
    /**
     * @brief Stop automatic cache refresh
     */
    void stop();
    
    /**
     * @brief Manually refresh cache immediately
     */
    void refresh();
    
    /**
     * @brief Check if an address is a local address
     * @param address Address to check
     * @return true if address is local, false otherwise
     */
    bool isLocalAddress(const QHostAddress& address) const;
    
    /**
     * @brief Get all local addresses
     * @return List of local IP addresses
     */
    QList<QHostAddress> localAddresses() const;
    
    /**
     * @brief Get all active network interfaces
     * @return List of active network interfaces
     */
    QList<QNetworkInterface> activeInterfaces() const;

signals:
    /**
     * @brief Emitted when cache is refreshed
     * @param interfaceCount Number of network interfaces found
     * @param addressCount Number of local addresses found
     */
    void cacheRefreshed(int interfaceCount, int addressCount);

private slots:
    /**
     * @brief Timer callback to refresh cache
     */
    void onRefreshTimer();

private:
    /**
     * @brief Update cache data (called by refresh)
     */
    void updateCache();

private:
    QTimer* m_refreshTimer;                    ///< Auto-refresh timer
    int m_refreshIntervalMs;                   ///< Refresh interval in milliseconds
    
    mutable QMutex m_mutex;                    ///< Mutex for thread-safe access
    QList<QNetworkInterface> m_interfaces;     ///< Cached network interfaces
    QList<QHostAddress> m_localAddresses;      ///< Cached local addresses
};

} // namespace communication
} // namespace flykylin
