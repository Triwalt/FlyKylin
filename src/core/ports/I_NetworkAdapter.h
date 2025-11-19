/**
 * @file I_NetworkAdapter.h
 * @brief 网络通信适配器接口（六边形架构 - Port）
 * 
 * 定义网络通信的抽象接口，与具体实现（Qt/Boost/标准库）解耦
 */

#ifndef I_NETWORKADAPTER_H
#define I_NETWORKADAPTER_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <optional>

namespace flykylin {
namespace ports {

/**
 * @brief 网络地址结构
 */
struct NetworkAddress {
    std::string ip;         ///< IP地址
    uint16_t port;          ///< 端口号
};

/**
 * @brief 网络消息结构
 */
struct NetworkMessage {
    NetworkAddress sender;          ///< 发送者地址
    std::vector<uint8_t> data;      ///< 消息数据
    uint64_t timestamp;             ///< 接收时间戳
};

/**
 * @class I_NetworkAdapter
 * @brief 网络通信适配器接口
 * 
 * 定义网络操作的抽象接口，支持：
 * - UDP广播/单播
 * - TCP连接
 * - 异步回调
 * 
 * 实现类：
 * - QtNetworkAdapter（使用Qt网络库）
 * - BoostNetworkAdapter（使用Boost.Asio）
 */
class I_NetworkAdapter {
public:
    virtual ~I_NetworkAdapter() = default;

    // ========== UDP操作 ==========
    
    /**
     * @brief 绑定UDP端口
     * @param port 端口号
     * @return 成功返回true
     */
    virtual bool bindUdp(uint16_t port) = 0;
    
    /**
     * @brief 发送UDP广播
     * @param data 数据内容
     * @param port 目标端口
     * @return 发送的字节数，失败返回0
     */
    virtual size_t sendUdpBroadcast(const std::vector<uint8_t>& data, uint16_t port) = 0;
    
    /**
     * @brief 发送UDP单播
     * @param data 数据内容
     * @param address 目标地址
     * @return 发送的字节数，失败返回0
     */
    virtual size_t sendUdpUnicast(const std::vector<uint8_t>& data, const NetworkAddress& address) = 0;
    
    /**
     * @brief 接收UDP数据（阻塞）
     * @param timeout_ms 超时时间（毫秒），0表示无限等待
     * @return 接收到的消息，超时返回nullopt
     */
    virtual std::optional<NetworkMessage> receiveUdp(uint32_t timeout_ms = 0) = 0;
    
    /**
     * @brief 设置UDP数据接收回调（异步模式）
     * @param callback 回调函数
     */
    virtual void setUdpReceiveCallback(std::function<void(const NetworkMessage&)> callback) = 0;

    // ========== TCP操作 ==========
    
    /**
     * @brief 监听TCP端口
     * @param port 端口号
     * @param max_connections 最大连接数
     * @return 成功返回true
     */
    virtual bool listenTcp(uint16_t port, int max_connections = 10) = 0;
    
    /**
     * @brief 连接到TCP服务器
     * @param address 服务器地址
     * @param timeout_ms 超时时间（毫秒）
     * @return 连接句柄，失败返回-1
     */
    virtual int connectTcp(const NetworkAddress& address, uint32_t timeout_ms = 5000) = 0;
    
    /**
     * @brief 发送TCP数据
     * @param connection_id 连接句柄
     * @param data 数据内容
     * @return 发送的字节数，失败返回0
     */
    virtual size_t sendTcp(int connection_id, const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief 接收TCP数据（阻塞）
     * @param connection_id 连接句柄
     * @param timeout_ms 超时时间（毫秒）
     * @return 接收到的数据，失败或超时返回空vector
     */
    virtual std::vector<uint8_t> receiveTcp(int connection_id, uint32_t timeout_ms = 0) = 0;
    
    /**
     * @brief 关闭TCP连接
     * @param connection_id 连接句柄
     */
    virtual void closeTcp(int connection_id) = 0;
    
    /**
     * @brief 设置TCP连接接受回调
     * @param callback 回调函数，参数为新连接的句柄
     */
    virtual void setTcpAcceptCallback(std::function<void(int connection_id, const NetworkAddress&)> callback) = 0;
    
    /**
     * @brief 设置TCP数据接收回调
     * @param callback 回调函数
     */
    virtual void setTcpReceiveCallback(std::function<void(int connection_id, const std::vector<uint8_t>&)> callback) = 0;
    
    /**
     * @brief 设置TCP连接断开回调
     * @param callback 回调函数
     */
    virtual void setTcpDisconnectCallback(std::function<void(int connection_id)> callback) = 0;

    // ========== 通用操作 ==========
    
    /**
     * @brief 获取本机所有IP地址
     * @return IP地址列表
     */
    virtual std::vector<std::string> getLocalIpAddresses() const = 0;
    
    /**
     * @brief 检查网络是否可用
     * @return 可用返回true
     */
    virtual bool isNetworkAvailable() const = 0;
};

} // namespace ports
} // namespace flykylin

#endif // I_NETWORKADAPTER_H
