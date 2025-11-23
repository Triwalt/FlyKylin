#include "TcpServer.h"

#include "TcpConnectionManager.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QDebug>

namespace flykylin {
namespace communication {

TcpServer::TcpServer(QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_listenPort(0)
{
    connect(m_server, &QTcpServer::newConnection,
            this, &TcpServer::onNewConnection);
}

TcpServer::~TcpServer()
{
    stop();
}

bool TcpServer::start(quint16 port)
{
    if (m_server->isListening()) {
        return true;
    }

    quint16 requestedPort = port;

    if (!m_server->listen(QHostAddress::AnyIPv4, requestedPort)) {
        qWarning() << "[TcpServer] Failed to listen on port" << requestedPort
                   << "error:" << m_server->errorString();

        if (requestedPort != 0) {
            if (!m_server->listen(QHostAddress::AnyIPv4, 0)) {
                qCritical() << "[TcpServer] Failed to listen on fallback port 0";
                return false;
            }
        } else {
            return false;
        }
    }

    m_listenPort = static_cast<quint16>(m_server->serverPort());

    qInfo() << "[TcpServer] Listening for incoming connections on port" << m_listenPort;
    return true;
}

void TcpServer::stop()
{
    if (!m_server->isListening()) {
        return;
    }

    qInfo() << "[TcpServer] Stopping server on port" << m_listenPort;
    m_server->close();
    m_listenPort = 0;
}

bool TcpServer::isListening() const
{
    return m_server->isListening();
}

quint16 TcpServer::listenPort() const
{
    return m_listenPort;
}

void TcpServer::onNewConnection()
{
    auto* manager = TcpConnectionManager::instance();

    while (m_server->hasPendingConnections()) {
        QTcpSocket* socket = m_server->nextPendingConnection();
        if (!socket) {
            continue;
        }

        QHostAddress peerAddress = socket->peerAddress();
        quint16 peerPort = static_cast<quint16>(socket->peerPort());
        QString peerId = QStringLiteral("%1:%2").arg(peerAddress.toString()).arg(peerPort);

        qInfo() << "[TcpServer] Incoming connection from" << peerId;

        if (manager) {
            manager->addIncomingConnection(peerId, socket);
        } else {
            qWarning() << "[TcpServer] TcpConnectionManager instance is null, closing socket";
            socket->close();
            socket->deleteLater();
        }
    }
}

} // namespace communication
} // namespace flykylin
