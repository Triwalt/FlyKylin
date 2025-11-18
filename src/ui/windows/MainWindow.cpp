/**
 * @file MainWindow.cpp
 * @brief 应用程序主窗口实现
 */

#include "MainWindow.h"
#include "../../core/communication/PeerDiscovery.h"
#include "../viewmodels/PeerListViewModel.h"
#include "../widgets/PeerListWidget.h"
#include <QDebug>
#include <QMessageBox>

namespace flykylin {
namespace ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi();
    setupServices();
    connectSignals();
    
    qDebug() << "[MainWindow] Initialized";
}

MainWindow::~MainWindow()
{
    // 停止服务
    if (m_peerDiscovery) {
        m_peerDiscovery->stop();
    }
    
    qDebug() << "[MainWindow] Destroyed";
}

void MainWindow::setupUi()
{
    setWindowTitle("FlyKylin P2P Chat - Sprint 1 Demo");
    setMinimumSize(400, 600);
    resize(500, 700);
    
    // 创建中央控件
    m_centralWidget = new QWidget(this);
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);
    
    // 标题
    m_titleLabel = new QLabel("FlyKylin P2P聊天应用", m_centralWidget);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_titleLabel);
    
    // ViewModel和用户列表
    m_peerListViewModel = new PeerListViewModel(this);
    m_peerListWidget = new PeerListWidget(m_peerListViewModel, m_centralWidget);
    m_mainLayout->addWidget(m_peerListWidget);
    
    // 状态标签
    m_statusLabel = new QLabel("正在启动服务...", m_centralWidget);
    m_statusLabel->setStyleSheet("color: blue; padding: 5px; background-color: #f0f0f0;");
    m_statusLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_statusLabel);
    
    m_centralWidget->setLayout(m_mainLayout);
    setCentralWidget(m_centralWidget);
}

void MainWindow::setupServices()
{
    // 创建UDP节点发现服务
    m_peerDiscovery = std::make_unique<flykylin::core::PeerDiscovery>();
    
    // 连接PeerDiscovery信号到ViewModel
    connect(m_peerDiscovery.get(), &flykylin::core::PeerDiscovery::peerDiscovered,
            m_peerListViewModel, &PeerListViewModel::onPeerDiscovered);
    
    connect(m_peerDiscovery.get(), &flykylin::core::PeerDiscovery::peerOffline,
            m_peerListViewModel, &PeerListViewModel::onPeerOffline);
    
    connect(m_peerDiscovery.get(), &flykylin::core::PeerDiscovery::peerHeartbeat,
            m_peerListViewModel, &PeerListViewModel::onPeerHeartbeat);
    
    // 启动服务
    bool started = m_peerDiscovery->start(kUdpPort, kTcpPort);
    
    if (started) {
        QString statusText = QString("✓ 服务启动成功 | UDP端口: %1 | TCP端口: %2")
                                .arg(kUdpPort)
                                .arg(kTcpPort);
        m_statusLabel->setText(statusText);
        m_statusLabel->setStyleSheet("color: green; padding: 5px; background-color: #e8f5e9;");
        
        qDebug() << "[MainWindow] Services started successfully";
    } else {
        m_statusLabel->setText("✗ 服务启动失败，请检查端口是否被占用");
        m_statusLabel->setStyleSheet("color: red; padding: 5px; background-color: #ffebee;");
        
        QMessageBox::warning(this, "启动失败",
            QString("无法启动P2P服务。\n\nUDP端口: %1\nTCP端口: %2\n\n请确保端口未被占用。")
                .arg(kUdpPort)
                .arg(kTcpPort));
        
        qCritical() << "[MainWindow] Failed to start services";
    }
}

void MainWindow::connectSignals()
{
    // 连接用户列表信号
    connect(m_peerListWidget, &PeerListWidget::userSelected,
            this, &MainWindow::onUserSelected);
    
    connect(m_peerListWidget, &PeerListWidget::userDoubleClicked,
            [this](const QString& userId) {
                qDebug() << "[MainWindow] User double-clicked:" << userId;
                // TODO: 打开聊天窗口（US-002实现）
                QMessageBox::information(this, "功能提示",
                    QString("即将与用户 %1 聊天\n（US-002: TCP长连接功能开发中）").arg(userId));
            });
}

void MainWindow::onUserSelected(const QString& userId)
{
    qDebug() << "[MainWindow] Current selected user:" << userId;
    
    // TODO: 更新UI显示选中用户信息
    // 当前Sprint 1仅实现节点发现和用户列表，后续Sprint实现聊天功能
}

} // namespace ui
} // namespace flykylin
