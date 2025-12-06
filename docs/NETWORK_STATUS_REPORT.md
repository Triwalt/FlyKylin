# FlyKylin 网络配置与运行状况报告

**日期**: 2024-12-06  
**状态**: ✅ 正常通信

---

## 1. 网络拓扑

```
┌─────────────────────┐         有线直连          ┌─────────────────────┐
│   PC (Windows)      │◄──────────────────────────►│   板端 (RK3566)     │
│   192.168.100.1     │      192.168.100.x/24      │   192.168.100.2     │
│   (以太网 2)        │                            │   (eth0)            │
└─────────────────────┘                            └─────────────────────┘
         │                                                  │
         │ WiFi: 172.26.103.217                             │ WiFi: 172.26.26.28
         │ (WLAN)                                           │ (wlan0)
         └──────────────────────────────────────────────────┘
                        同一WiFi网段 172.26.x.x/17
```

---

## 2. PC端 (Windows) 配置

### 2.1 系统信息
- **操作系统**: Windows
- **用户ID**: `6f300bc3-ca75-405e-9a24-f4306adaea25:45679`
- **用户名**: 33_Test

### 2.2 网络接口
| 接口名称 | IP地址 | 用途 |
|---------|--------|------|
| 以太网 2 | 192.168.100.1 | 与板端直连 |
| WLAN | 172.26.103.217 | WiFi网络 |
| vEthernet (WSL) | 172.27.208.1 | WSL虚拟网络 |
| vEthernet (Default Switch) | 172.19.0.1 | Hyper-V |
| Tailscale | 169.254.83.107 | VPN |

### 2.3 端口状态
| 协议 | 端口 | 状态 | 用途 |
|------|------|------|------|
| TCP | 45679 | LISTENING | 消息传输 |
| UDP | 45678 | LISTENING | 节点发现 |
| TCP | 45679 | ESTABLISHED → 192.168.100.2:47564 | 与板端连接 |

### 2.4 防火墙规则
- `flykylin` - 入站允许 (多条)
- `FlyKylin UDP Discovery` - UDP 45678 入站允许
- `flykylin_tests` - 入站允许

### 2.5 网络配置文件
| 网络名称 | 接口 | 类别 |
|---------|------|------|
| 未识别的网络 | 以太网 2 | Public |
| SYSU-SECURE 19 | WLAN | Public |
| Tailscale | Tailscale | Private |

---

## 3. 板端 (RK3566) 配置

### 3.1 系统信息
- **操作系统**: Ubuntu 20.04 (Linux 5.10.198 aarch64)
- **CPU**: Rockchip RK3566
- **内存**: 956MB 物理内存 + 4GB Swap
- **用户ID**: `45f81eb5-1a60-43df-8b55-d42ba3ce2365:45679`
- **用户名**: kylin

### 3.2 内存状态
| 类型 | 总量 | 已用 | 可用 |
|------|------|------|------|
| 物理内存 | 956MB | ~680MB | ~170MB |
| Swap | 4GB | ~1.5MB | ~4GB |

### 3.3 网络接口
| 接口名称 | IP地址 | 用途 |
|---------|--------|------|
| eth0 | 192.168.100.2/24 | 与PC直连 (静态IP) |
| wlan0 | 172.26.26.28/17 | WiFi网络 (DHCP) |

### 3.4 端口状态
| 协议 | 端口 | 状态 | 用途 |
|------|------|------|------|
| TCP | 45679 | LISTENING | 消息传输 |
| UDP | 45678 | LISTENING | 节点发现 |
| TCP | 47564 | ESTABLISHED → 192.168.100.1:45679 | 与PC连接 |

### 3.5 FlyKylin进程
- **PID**: 1669
- **CPU**: 3.0%
- **内存**: 8.4% (~83MB)

---

## 4. 通信状态分析

### 4.1 节点发现 (UDP 45678)
- ✅ PC端发送广播到所有接口
- ✅ 板端发送广播到 eth0 和 wlan0
- ✅ 板端发送直接消息到已知对端 192.168.100.1
- ✅ 双方能收到对方的心跳消息

### 4.2 TCP连接 (端口 45679)
- ✅ 连接已建立: `192.168.100.2:47564 ↔ 192.168.100.1:45679`
- ✅ 连接状态: ESTABLISHED
- ✅ 消息可以正常收发

### 4.3 延迟测试
- 有线直连延迟: <1ms

---

## 5. 已解决的问题

### 5.1 防火墙配置问题
**问题**: PC端UDP 45678入站被阻止，导致PC端无法发现板端  
**原因**: "FlyKylin UDP Discovery"规则端口配置错误(45679而非45678)  
**解决**: 修正防火墙规则端口为45678

### 5.2 板端显示问题
**问题**: 板端程序启动后黑屏或无GUI显示  
**原因**: 使用了linuxfb平台插件而非X11  
**解决**: 设置 `DISPLAY=:0` 在X11桌面环境下启动

### 5.3 板端内存不足
**问题**: 板端只有956MB内存，程序运行时可能OOM  
**解决**: 配置4GB SD卡swap，开机自动启用

### 5.4 网络配置持久化
**问题**: 板端eth0重启后丢失IP配置  
**解决**: 使用NetworkManager配置静态IP

### 5.5 TCP连接peerId映射问题 (2024-12-06 修复)
**问题**: TCP连接建立后，消息发送失败，提示"Not connected"  
**原因**: 
1. 板端`NetworkInterfaceCache`只检测到wlan0接口，没有检测到eth0
2. 入站连接使用临时peerId (IP:port)，握手后需要更新为真正的userId
3. `handleHandshakeRequest`中的`peerIdUpdated`信号没有正确触发peerId映射更新

**根因分析**:
- 板端应用启动时eth0接口可能还未完全就绪
- Qt的`QNetworkInterface::allInterfaces()`在某些情况下可能遗漏接口
- 握手请求中的`user_id`字段为空导致peerId无法更新

**解决方案**:
1. 在`TcpConnection::handleHandshakeRequest`中添加详细日志，显示收到的user_id
2. 在`TcpConnectionManager::onPeerIdUpdated`中添加调试日志
3. 确保板端应用在网络接口完全就绪后启动
4. 重新编译并部署板端应用

**验证结果**:
- ✅ 板端现在检测到2个网络接口 (eth0 + wlan0)
- ✅ peerId正确从临时IP:port更新为userId
- ✅ 双向TCP消息收发正常

---

## 6. 优化建议

### 6.1 性能优化
1. **减少广播频率**: 当前每3秒发送一次心跳，可考虑在连接稳定后降低频率
2. **优化日志级别**: 生产环境可降低日志详细程度减少I/O

### 6.2 稳定性优化
1. **TCP重连策略**: 当前使用指数退避，建议添加最大重试次数限制
2. **心跳超时**: 建议配置合理的心跳超时时间(如30秒)

### 6.3 安全优化
1. **防火墙规则**: 建议限制规则只允许特定IP范围
2. **网络隔离**: 考虑将"以太网 2"设为Private网络

### 6.4 板端启动脚本
建议创建开机自启动脚本:
```bash
#!/bin/bash
# /home/kylin/FlyKylinApp/start.sh
export DISPLAY=:0
export LD_LIBRARY_PATH=/home/kylin/FlyKylinApp/lib
cd /home/kylin/FlyKylinApp/bin
./FlyKylin > /tmp/flykylin.log 2>&1
```

---

## 7. 快速参考

### 启动命令

**PC端 (Windows)**:
```powershell
cd E:\Project\FlyKylin\build\bin\Release
.\FlyKylin.exe
```

**板端 (RK3566)**:
```bash
export DISPLAY=:0
export LD_LIBRARY_PATH=/home/kylin/FlyKylinApp/lib
cd /home/kylin/FlyKylinApp/bin
./FlyKylin
```

### 网络端口
- **UDP 45678**: 节点发现/广播
- **TCP 45679**: 消息传输

### 关键文件路径
- PC端日志: `E:\Project\FlyKylin\flykylin_log.txt`
- 板端日志: `/tmp/flykylin.log`
- 板端程序: `/home/kylin/FlyKylinApp/bin/FlyKylin`
- 板端库文件: `/home/kylin/FlyKylinApp/lib/`
