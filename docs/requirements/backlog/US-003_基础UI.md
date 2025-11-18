# User Story: US-003 基础UI-在线用户列表

**标题**: US-003 实现在线用户列表UI

**优先级**: High  
**故事点**: 5  
**Sprint**: Sprint 1  
**状态**: 待开发

---

## User Story

**作为** 企业局域网用户  
**我希望** 看到当前在线的所有用户列表  
**以便** 选择某个用户开始聊天

## 验收标准

### AC1: 显示在线用户列表
- **Given** 应用启动并发现3个在线用户
- **When** 打开主窗口
- **Then** 在左侧面板显示在线用户列表
- **And** 每个用户显示：用户名、主机名、IP地址、在线状态（绿点）

### AC2: 实时更新用户上线
- **Given** 用户列表当前有2个在线用户
- **When** 用户C启动应用并广播上线
- **Then** 用户列表自动添加用户C
- **And** 播放上线提示音（可选）
- **And** 系统托盘显示"用户C已上线"通知

### AC3: 实时更新用户离线
- **Given** 用户列表当前有3个在线用户
- **When** 用户B关闭应用或断网
- **Then** 用户B在列表中状态变为灰色（离线）
- **And** 30秒后从列表中移除
- **And** 系统托盘显示"用户B已离线"通知

### AC4: 选择用户打开聊天
- **Given** 用户列表显示5个在线用户
- **When** 双击"用户A"
- **Then** 在右侧打开与"用户A"的聊天窗口
- **And** 聊天窗口标题显示"与 用户A (192.168.1.100) 的对话"
- **And** 聊天窗口准备就绪可输入消息

### AC5: 搜索用户
- **Given** 用户列表有20个在线用户
- **When** 在搜索框输入"张三"
- **Then** 列表只显示用户名包含"张三"的用户
- **And** 清空搜索框后恢复显示所有用户

## 技术约束

### 平台支持
- Windows x64 (MSVC 2022)

### 性能目标
- UI响应时间 < 100ms
- 列表刷新帧率 ≥ 30fps
- 支持显示200+在线用户

### UI框架
- Qt 6.9.3 Widgets
- 使用QListView + QStandardItemModel
- MVVM模式：PeerListViewModel

### 依赖
- US-001 UDP节点发现（数据源）
- Qt Widgets模块

## 技术设计

### 涉及模块
- `src/ui/widgets/PeerListWidget.h/cpp`
- `src/ui/viewmodels/PeerListViewModel.h/cpp`
- `src/ui/main_window.ui`

### 核心类设计
```cpp
// PeerListViewModel: 用户列表视图模型
class PeerListViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStandardItemModel* peerListModel READ peerListModel CONSTANT)
    Q_PROPERTY(int onlineCount READ onlineCount NOTIFY onlineCountChanged)
    
public:
    explicit PeerListViewModel(QObject* parent = nullptr);
    
    QStandardItemModel* peerListModel() const { return m_model; }
    int onlineCount() const { return m_peers.count(); }
    
public slots:
    void onPeerDiscovered(const PeerNode& peer);
    void onPeerOffline(const QString& userId);
    void filterPeers(const QString& keyword);
    
signals:
    void onlineCountChanged();
    void peerSelected(const PeerNode& peer);
    
private:
    QStandardItemModel* m_model;
    QMap<QString, PeerNode> m_peers;
    QString m_filterKeyword;
    
    void updateModel();
    QStandardItem* createPeerItem(const PeerNode& peer);
};

// PeerListWidget: 用户列表UI控件
class PeerListWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit PeerListWidget(QWidget* parent = nullptr);
    void setViewModel(PeerListViewModel* viewModel);
    
signals:
    void peerDoubleClicked(const PeerNode& peer);
    
private:
    QListView* m_listView;
    QLineEdit* m_searchEdit;
    QLabel* m_onlineCountLabel;
    PeerListViewModel* m_viewModel;
};
```

### UI布局
```
+------------------------------------------+
| FlyKylin - 局域网即时通讯                 |
+------------------------------------------+
| [搜索用户...]                      |菜单| |
+------------------------------------------+
|                    |                     |
| 在线用户 (5)       |  聊天窗口区域       |
| +--------------+   |                     |
| | ● 张三      |   |                     |
| |   192.168.1.10  |                     |
| +--------------+   |                     |
| | ● 李四      |   |                     |
| |   192.168.1.20  |                     |
| +--------------+   |                     |
| | ○ 王五(离线)|   |                     |
| +--------------+   |                     |
|                    |                     |
+------------------------------------------+
```

## 测试策略

### 单元测试
- ViewModel的增删改查逻辑
- 搜索过滤功能
- 在线状态变化

### UI测试
- 显示10个用户，验证UI正常
- 动态添加/删除用户，验证刷新
- 搜索功能测试

### 集成测试
- 与US-001集成，真实节点发现
- 双击用户触发聊天窗口（Mock）

## 风险与缓解

### 风险1: 大量用户时UI卡顿
- **缓解**: 使用虚拟列表（QListView已优化）
- **测试**: 压力测试200+用户

### 风险2: 状态更新不及时
- **缓解**: 使用Qt信号槽实时更新
- **验证**: 集成测试验证延迟

## Definition of Done

- [ ] `PeerListViewModel`和`PeerListWidget`实现完成
- [ ] UI布局符合设计
- [ ] 单元测试覆盖率 ≥ 70%
- [ ] 与US-001集成测试通过
- [ ] 性能目标达成（<100ms响应）
- [ ] Code Review通过
- [ ] UI截图更新到文档

## 相关Story
- US-001 UDP节点发现（数据源）
- US-002 TCP长连接（双击后建立连接）
- US-004 聊天窗口UI（Sprint 2）
