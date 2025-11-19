/**
 * @file tcp_connection_example.cpp
 * @brief Example demonstrating TCP connection manager usage
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include "core/communication/TcpConnectionManager.h"

using namespace flykylin::communication;

class TcpExample : public QObject {
    Q_OBJECT
    
public:
    TcpExample(QObject* parent = nullptr) : QObject(parent) {
        // Get connection manager instance
        m_manager = TcpConnectionManager::instance();
        
        // Connect signals
        connect(m_manager, &TcpConnectionManager::connectionStateChanged,
                this, &TcpExample::onConnectionStateChanged);
        connect(m_manager, &TcpConnectionManager::messageReceived,
                this, &TcpExample::onMessageReceived);
        connect(m_manager, &TcpConnectionManager::messageSent,
                this, &TcpExample::onMessageSent);
        connect(m_manager, &TcpConnectionManager::messageFailed,
                this, &TcpExample::onMessageFailed);
    }
    
    void run() {
        qInfo() << "=== TCP Connection Manager Example ===";
        qInfo() << "";
        
        // Example 1: Connect to a peer
        qInfo() << "Example 1: Connecting to peer...";
        m_manager->connectToPeer("peer_001", "127.0.0.1", 8888);
        
        // Example 2: Send a message after 2 seconds
        QTimer::singleShot(2000, this, [this]() {
            qInfo() << "";
            qInfo() << "Example 2: Sending message...";
            QByteArray testData = "Hello from FlyKylin TCP!";
            m_manager->sendMessage("peer_001", testData, MessageQueue::Priority::High);
        });
        
        // Example 3: Check connection count after 3 seconds
        QTimer::singleShot(3000, this, [this]() {
            qInfo() << "";
            qInfo() << "Example 3: Active connections:" << m_manager->activeConnectionCount();
        });
        
        // Example 4: Disconnect after 5 seconds
        QTimer::singleShot(5000, this, [this]() {
            qInfo() << "";
            qInfo() << "Example 4: Disconnecting...";
            m_manager->disconnectFromPeer("peer_001");
        });
        
        // Exit after 6 seconds
        QTimer::singleShot(6000, []() {
            qInfo() << "";
            qInfo() << "=== Example finished ===";
            QCoreApplication::quit();
        });
    }
    
private slots:
    void onConnectionStateChanged(QString peerId, ConnectionState state, QString reason) {
        qInfo() << "[STATE]" << peerId << "state =" << static_cast<int>(state) << "reason:" << reason;
    }
    
    void onMessageReceived(QString peerId, QByteArray data) {
        qInfo() << "[RECEIVED]" << peerId << "data:" << QString::fromUtf8(data);
    }
    
    void onMessageSent(QString peerId, quint64 messageId) {
        qInfo() << "[SENT]" << peerId << "message" << messageId << "sent successfully";
    }
    
    void onMessageFailed(QString peerId, quint64 messageId, QString error) {
        qWarning() << "[FAILED]" << peerId << "message" << messageId << "failed:" << error;
    }
    
private:
    TcpConnectionManager* m_manager;
};

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    TcpExample example;
    
    // Run after event loop starts
    QTimer::singleShot(0, &example, &TcpExample::run);
    
    return app.exec();
}

#include "tcp_connection_example.moc"
