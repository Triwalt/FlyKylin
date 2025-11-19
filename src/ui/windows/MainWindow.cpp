/**
 * @file MainWindow.cpp
 * @brief 应用程序主窗口实现
 */

#include "MainWindow.h"
#include "ChatWindow.h"
#include "../../core/communication/PeerDiscovery.h"
#include "../../core/services/LocalEchoService.h"
#include "../viewmodels/PeerListViewModel.h"
#include "../widgets/PeerListWidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QMap>

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
    
    // 启用本地回环模式（用于开发测试，可以发现本机节点）
    m_peerDiscovery->setLoopbackEnabled(true);
    qInfo() << "[MainWindow] Loopback mode enabled for development testing";
    
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
    
    // Add Echo Bot as virtual peer for local testing
    core::PeerNode echoBot;
    echoBot.setUserId(services::LocalEchoService::getEchoBotId());
    echoBot.setUserName(services::LocalEchoService::getEchoBotName());
    echoBot.setHostName("localhost");
    echoBot.setIpAddress("127.0.0.1");
    echoBot.setTcpPort(0);  // No real TCP connection
    echoBot.setOnline(true);
    echoBot.setLastSeen(QDateTime::currentDateTime());
    
    m_peerListViewModel->onPeerDiscovered(echoBot);
    qInfo() << "[MainWindow] Added Echo Bot for local testing";
}

void MainWindow::connectSignals()
{
    // 连接用户列表信号
    connect(m_peerListWidget, &PeerListWidget::userSelected,
            this, &MainWindow::onUserSelected);
    
    connect(m_peerListWidget, &PeerListWidget::userDoubleClicked,
            [this](const QString& userId) {
                qDebug() << "[MainWindow] User double-clicked:" << userId;
                this->openChatWindow(userId);
            });
}

void MainWindow::onUserSelected(const QString& userId)
{
    qDebug() << "[MainWindow] Current selected user:" << userId;
    
    // TODO: 更新UI显示选中用户信息
    // 当前Sprint 1仅实现节点发现和用户列表，后续Sprint实现聊天功能
}

void MainWindow::openChatWindow(const QString& userId)
{
    // Check if window already exists
    if (m_chatWindows.contains(userId)) {
        // Bring existing window to front
        ChatWindow* existingWindow = m_chatWindows[userId];
        existingWindow->show();
        existingWindow->raise();
        existingWindow->activateWindow();
        qDebug() << "[MainWindow] Brought existing chat window to front for" << userId;
        return;
    }
    
    // Get peer name (for now, use userId as name)
    // TODO: Get actual peer name from PeerListViewModel when method is implemented
    QString peerName = userId;
    
    // Create new chat window
    ChatWindow* chatWindow = new ChatWindow();
    chatWindow->setPeer(userId, peerName);
    chatWindow->setAttribute(Qt::WA_DeleteOnClose); // Auto-delete when closed
    
    // Connect closed signal
    connect(chatWindow, &ChatWindow::closed,
            [this, userId]() {
                this->onChatWindowClosed(userId);
            });
    
    // Store and show window
    m_chatWindows[userId] = chatWindow;
    chatWindow->show();
    
    qInfo() << "[MainWindow] Opened new chat window for" << peerName << "(" << userId << ")";
}

void MainWindow::onChatWindowClosed(const QString& userId)
{
    // Remove from map (window will auto-delete)
    if (m_chatWindows.remove(userId)) {
        qDebug() << "[MainWindow] Chat window closed for" << userId;
    }
}

} // namespace ui
} // namespace flykylin
