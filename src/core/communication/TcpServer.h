#pragma once

#include <QObject>
#include <QTcpServer>

namespace flykylin {
namespace communication {

class TcpServer : public QObject {
    Q_OBJECT

public:
    explicit TcpServer(QObject* parent = nullptr);
    ~TcpServer() override;

    bool start(quint16 port);
    void stop();

    bool isListening() const;
    quint16 listenPort() const;

private slots:
    void onNewConnection();

private:
    QTcpServer* m_server;
    quint16 m_listenPort;
};

} // namespace communication
} // namespace flykylin
