#include "UdpDiscoveryAdapter.h"
#include <QHostAddress>
#include <QDebug>

namespace flykylin::core::adapters {

UdpDiscoveryAdapter::UdpDiscoveryAdapter(uint16_t port)
    : m_port(port)
{
}

UdpDiscoveryAdapter::~UdpDiscoveryAdapter() {
    stop();
}

bool UdpDiscoveryAdapter::start() {
    if (m_running) return true;

    m_socket = std::make_unique<QUdpSocket>();
    if (!m_socket->bind(QHostAddress::Any, m_port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qCritical() << "Failed to bind UDP socket on port" << m_port;
        return false;
    }

    m_running = true;
    // 启动接收循环（fire and forget，实际项目中应管理生命周期）
    // 注意：这里需要一个机制来保持协程运行，或者将其挂载到某个上下文
    // receiveLoop(); 
    
    qDebug() << "UDP Discovery Adapter started on port" << m_port;
    return true;
}

void UdpDiscoveryAdapter::stop() {
    m_running = false;
    if (m_socket) {
        m_socket->close();
        m_socket.reset();
    }
}

QCoro::Task<bool> UdpDiscoveryAdapter::sendMessageAsync(const std::string& targetId, const std::string& data) {
    // UDP发现通常不针对特定ID发送单播，除非已知IP
    // 这里简化处理
    co_return false;
}

QCoro::Task<bool> UdpDiscoveryAdapter::broadcastAsync(const std::string& data) {
    if (!m_socket) co_return false;

    QByteArray datagram(data.data(), data.size());
    qint64 bytesWritten = m_socket->writeDatagram(datagram, QHostAddress::Broadcast, m_port);
    
    co_return bytesWritten == datagram.size();
}

QCoro::Task<void> UdpDiscoveryAdapter::receiveLoop() {
    while (m_running && m_socket) {
        // 使用 QCoro 等待数据报
        // 注意：需要确保 qcoro::network 模块正确链接
        // auto datagram = co_await m_socket->readDatagram();
        // 处理 datagram...
        co_return; // 占位
    }
}

} // namespace flykylin::core::adapters
