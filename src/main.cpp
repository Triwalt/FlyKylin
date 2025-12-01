/**
 * @file main.cpp
 * @brief FlyKylin P2P聊天应用入口
 */

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCommandLineParser>
#include <QtQuickControls2/QQuickStyle>
#include <memory>
#include "ui/windows/MainWindow.h"
#include "ui/viewmodels/PeerListViewModel.h"
#include "ui/viewmodels/ChatViewModel.h"
#include "ui/viewmodels/SettingsViewModel.h"
#include "ui/viewmodels/GlobalSearchViewModel.h"
#include "core/communication/PeerDiscovery.h"
#include "core/communication/TcpServer.h"
#include "core/communication/TcpConnectionManager.h"
#include "core/config/UserProfile.h"

// Uncomment to switch to QML UI
#define USE_QML_UI

namespace {
constexpr quint16 kUdpPort = 45678;
constexpr quint16 kTcpPort = 45679;
}

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName("FlyKylin");
    QCoreApplication::setOrganizationDomain("flykylin.local");
    QCoreApplication::setApplicationName("FlyKylin");
    QApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    // Command line options
    QCommandLineParser parser;
    parser.setApplicationDescription("FlyKylin P2P Chat");
    parser.addHelpOption();

    QCommandLineOption tcpPortOption(
        {"p", "tcp-port"},
        QStringLiteral("TCP port to listen on (0 = auto, default = %1)").arg(kTcpPort),
        QStringLiteral("port"));
    parser.addOption(tcpPortOption);

    parser.process(app);

    quint16 tcpPort = kTcpPort;
    if (parser.isSet(tcpPortOption)) {
        bool ok = false;
        const int value = parser.value(tcpPortOption).toInt(&ok);
        if (!ok || value < 0 || value > 65535) {
            qWarning() << "[main] Invalid --tcp-port value" << parser.value(tcpPortOption)
                       << ", using default" << kTcpPort;
        } else {
            tcpPort = static_cast<quint16>(value);
        }
    }

#ifdef USE_QML_UI
    QQmlApplicationEngine engine;

    // Instantiate core services
    auto tcpServer = std::make_unique<flykylin::communication::TcpServer>();
    bool serverStarted = tcpServer->start(tcpPort);
    quint16 effectiveTcpPort = serverStarted ? tcpServer->listenPort() : 0;

    if (effectiveTcpPort != 0) {
        flykylin::core::UserProfile::instance().setInstanceSuffix(QString(":%1").arg(effectiveTcpPort));
    }

    auto peerDiscovery = std::make_unique<flykylin::core::PeerDiscovery>();

    // Enable loopback for local development and integrate with TcpConnectionManager
    peerDiscovery->setLoopbackEnabled(true);
    flykylin::communication::TcpConnectionManager::instance()->setupPeerDiscovery(peerDiscovery.get());

    bool discoveryStarted = peerDiscovery->start(kUdpPort, effectiveTcpPort);
    Q_UNUSED(discoveryStarted);

    // Instantiate ViewModels
    flykylin::ui::PeerListViewModel peerListViewModel;
    flykylin::ui::ChatViewModel chatViewModel;
    flykylin::ui::SettingsViewModel settingsViewModel;
    flykylin::ui::GlobalSearchViewModel globalSearchViewModel;

    // 在启动时加载历史会话，让“会话”页能显示离线用户的历史对话
    peerListViewModel.loadHistoricalSessions();

    // Connect Services to ViewModels
    QObject::connect(peerDiscovery.get(), &flykylin::core::PeerDiscovery::peerDiscovered,
                     &peerListViewModel, &flykylin::ui::PeerListViewModel::onPeerDiscovered);
    QObject::connect(peerDiscovery.get(), &flykylin::core::PeerDiscovery::peerOffline,
                     &peerListViewModel, &flykylin::ui::PeerListViewModel::onPeerOffline);
    QObject::connect(peerDiscovery.get(), &flykylin::core::PeerDiscovery::peerHeartbeat,
                     &peerListViewModel, &flykylin::ui::PeerListViewModel::onPeerHeartbeat);

    // Connect Peer Selection to Chat
    QObject::connect(&peerListViewModel, &flykylin::ui::PeerListViewModel::peerSelected,
                     [&chatViewModel](const flykylin::core::PeerNode& peer) {
        chatViewModel.setCurrentPeer(peer.userId(), peer.userName());
    });

    // Register ViewModels as Context Properties
    engine.rootContext()->setContextProperty("peerListViewModel", &peerListViewModel);
    engine.rootContext()->setContextProperty("chatViewModel", &chatViewModel);
    engine.rootContext()->setContextProperty("settingsViewModel", &settingsViewModel);
    engine.rootContext()->setContextProperty("globalSearchViewModel", &globalSearchViewModel);

    const QUrl url(u"qrc:/FlyKylin/src/ui/qml/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    // Load QML from file system for development (no qrc needed yet)
    engine.load(QUrl::fromLocalFile("src/ui/qml/Main.qml"));
#else
    // 创建并显示主窗口
    flykylin::ui::MainWindow mainWindow;
    mainWindow.show();
#endif
    
    return app.exec();
}
