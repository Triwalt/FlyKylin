/**
 * @file MainWindow.h
 * @brief 应用程序主窗口
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <memory>

// 前向声明
namespace flykylin {
namespace core {
    class PeerDiscovery;
}
namespace ui {
    class PeerListViewModel;
    class PeerListWidget;
}
}

namespace flykylin {
namespace ui {

/**
 * @class MainWindow
 * @brief 应用程序主窗口
 * 
 * 功能：
 * - 启动UDP节点发现服务
 * - 显示在线用户列表
 * - 提供基本的UI框架
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MainWindow();

private:
    /**
     * @brief 初始化UI
     */
    void setupUi();

    /**
     * @brief 初始化服务
     */
    void setupServices();

    /**
     * @brief 连接信号槽
     */
    void connectSignals();

private slots:
    /**
     * @brief 处理用户被选中
     * @param userId 用户ID
     */
    void onUserSelected(const QString& userId);

private:
    // 核心服务
    std::unique_ptr<flykylin::core::PeerDiscovery> m_peerDiscovery;

    // UI组件
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    PeerListViewModel* m_peerListViewModel;
    PeerListWidget* m_peerListWidget;
    QLabel* m_statusLabel;

    // 常量
    static constexpr quint16 kUdpPort = 45678;
    static constexpr quint16 kTcpPort = 45679;
};

} // namespace ui
} // namespace flykylin

#endif // MAINWINDOW_H
