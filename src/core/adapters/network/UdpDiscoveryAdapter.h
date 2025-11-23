#pragma once

#include "../../interfaces/I_NetworkAdapter.h"
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <qcoro/network/qudpsocket.h>

namespace flykylin::core::adapters {

/**
 * @brief UDP广播适配器实现
 * 使用 Qt Network 和 QCoro 实现
 */
class UdpDiscoveryAdapter : public I_NetworkAdapter {
public:
    UdpDiscoveryAdapter(uint16_t port = 55555);
    ~UdpDiscoveryAdapter() override;

    bool start() override;
    void stop() override;
    
    QCoro::Task<bool> sendMessageAsync(const std::string& targetId, const std::string& data) override;
    QCoro::Task<bool> broadcastAsync(const std::string& data) override;

    /**
     * @brief 接收循环（协程）
     */
    QCoro::Task<void> receiveLoop();

private:
    std::unique_ptr<QUdpSocket> m_socket;
    uint16_t m_port;
    bool m_running = false;
};

} // namespace flykylin::core::adapters
