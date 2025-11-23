/**
 * @file NetworkInterfaceCache.cpp
 * @brief Network interface caching implementation
 * @author FlyKylin Development Team
 * @date 2024-11-20
 */

#include "NetworkInterfaceCache.h"
#include <QMutexLocker>
#include <QDebug>

namespace flykylin {
namespace communication {

NetworkInterfaceCache::NetworkInterfaceCache(QObject* parent, int refreshIntervalMs)
    : QObject(parent)
    , m_refreshTimer(new QTimer(this))
    , m_refreshIntervalMs(refreshIntervalMs)
{
    // Connect timer
    connect(m_refreshTimer, &QTimer::timeout,
            this, &NetworkInterfaceCache::onRefreshTimer);
    
    // Initial cache population
    refresh();
    
    qInfo() << "[NetworkInterfaceCache] Initialized with refresh interval" 
            << refreshIntervalMs << "ms";
}

NetworkInterfaceCache::~NetworkInterfaceCache()
{
    stop();
    qInfo() << "[NetworkInterfaceCache] Destroyed";
}

void NetworkInterfaceCache::start()
{
    if (m_refreshTimer->isActive()) {
        qDebug() << "[NetworkInterfaceCache] Already started";
        return;
    }
    
    m_refreshTimer->start(m_refreshIntervalMs);
    qInfo() << "[NetworkInterfaceCache] Auto-refresh started";
}

void NetworkInterfaceCache::stop()
{
    if (!m_refreshTimer->isActive()) {
        return;
    }
    
    m_refreshTimer->stop();
    qInfo() << "[NetworkInterfaceCache] Auto-refresh stopped";
}

void NetworkInterfaceCache::refresh()
{
    updateCache();
}

bool NetworkInterfaceCache::isLocalAddress(const QHostAddress& address) const
{
    QMutexLocker locker(&m_mutex);
    
    // Check if address is in cached local addresses
    return m_localAddresses.contains(address);
}

QList<QHostAddress> NetworkInterfaceCache::localAddresses() const
{
    QMutexLocker locker(&m_mutex);
    return m_localAddresses;
}

QList<QNetworkInterface> NetworkInterfaceCache::activeInterfaces() const
{
    QMutexLocker locker(&m_mutex);
    return m_interfaces;
}

void NetworkInterfaceCache::onRefreshTimer()
{
    qDebug() << "[NetworkInterfaceCache] Auto-refreshing cache";
    updateCache();
}

void NetworkInterfaceCache::updateCache()
{
    // Get all network interfaces (expensive operation)
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    
    QList<QNetworkInterface> activeInterfaces;
    QList<QHostAddress> localAddresses;
    
    // Filter active interfaces and collect addresses
    for (const QNetworkInterface& iface : allInterfaces) {
        // Skip loopback and inactive interfaces
        if (iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            continue;
        }
        
        if (!iface.flags().testFlag(QNetworkInterface::IsUp) ||
            !iface.flags().testFlag(QNetworkInterface::IsRunning)) {
            continue;
        }
        
        activeInterfaces.append(iface);
        
        // Collect all addresses from this interface
        for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
            QHostAddress addr = entry.ip();
            
            // Filter out link-local and other unwanted addresses
            if (addr.protocol() == QAbstractSocket::IPv4Protocol ||
                addr.protocol() == QAbstractSocket::IPv6Protocol) {
                
                // Skip link-local IPv6 addresses
                if (addr.protocol() == QAbstractSocket::IPv6Protocol &&
                    addr.isLinkLocal()) {
                    continue;
                }
                
                localAddresses.append(addr);
            }
        }
    }
    
    // Update cache atomically
    {
        QMutexLocker locker(&m_mutex);
        m_interfaces = activeInterfaces;
        m_localAddresses = localAddresses;
    }
    
    qDebug() << "[NetworkInterfaceCache] Cache updated:" 
             << activeInterfaces.size() << "interfaces,"
             << localAddresses.size() << "addresses";
    
    // Emit signal
    emit cacheRefreshed(activeInterfaces.size(), localAddresses.size());
}

} // namespace communication
} // namespace flykylin
