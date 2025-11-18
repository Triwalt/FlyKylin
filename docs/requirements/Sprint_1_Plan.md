# Sprint 1: P2P通信基础

**周期**: 2周  
**目标**: 实现局域网P2P节点发现和基础文本通信  
**状态**: 就绪

## Sprint目标

完成FlyKylin的核心P2P通信能力，实现Windows平台上两个客户端的1v1文本聊天。

## User Stories

| ID | 标题 | 优先级 | 故事点 | 负责人 |
|----|------|--------|--------|--------|
| US-001 | UDP节点发现 | Critical | 5 | TBD |
| US-002 | TCP长连接消息传输 | Critical | 8 | TBD |
| US-003 | 基础UI用户列表 | High | 5 | TBD |

**总故事点**: 18点

## Definition of Done

- [ ] 所有User Story验收标准通过
- [ ] 单元测试覆盖率 ≥ 70%
- [ ] 2个Windows实例互相发现并可发送文本消息
- [ ] 性能目标达成（节点发现<5秒，消息延迟<100ms）
- [ ] Code Review完成
- [ ] 技术文档更新

## 技术债务

- Protobuf需要完整安装（vcpkg）
- 需要实现基础日志系统
- 需要配置单元测试框架（GoogleTest）

## 风险

- **High**: Protobuf安装问题 - 使用vcpkg解决
- **Medium**: UDP广播被防火墙拦截 - 提供文档说明
- **Low**: 跨网段发现失败 - 文档说明限制

## Sprint Review计划

演示内容：
1. 启动2个FlyKylin实例
2. 自动发现并显示对方
3. 发送文本消息"Hello from Sprint 1!"
4. 接收并显示消息
