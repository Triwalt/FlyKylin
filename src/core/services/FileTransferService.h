#pragma once

#include <QObject>
#include <QMap>
#include <QString>

#include "core/models/Message.h"
#include "core/communication/TcpConnectionManager.h"

namespace flykylin {
namespace services {

class FileTransferService : public QObject {
    Q_OBJECT

public:
    explicit FileTransferService(QObject* parent = nullptr);
    ~FileTransferService() override;

    void sendImage(const QString& peerId, const QString& filePath);
    void sendFile(const QString& peerId, const QString& filePath);

    void handleIncomingTcpData(const QString& peerId, const QByteArray& data);
    void acceptTransfer(const QString& transferId, const QString& targetDirectory = QString());
    void rejectTransfer(const QString& transferId, const QString& reason = QString());

    void setAutoAcceptImages(bool enabled) { m_autoAcceptImages = enabled; }
    void setAutoAcceptFiles(bool enabled) { m_autoAcceptFiles = enabled; }

    void setDownloadDirectory(const QString& path);
    QString downloadDirectory() const;

signals:
    void messageCreated(const flykylin::core::Message& message);
    void transferFailed(QString transferId, QString error);
    void transferCompleted(QString transferId, const flykylin::core::Message& message);
    void incomingTransferRequested(QString transferId,
                                   QString peerId,
                                   const flykylin::core::Message& message);

private slots:
    void onTcpMessageReceived(QString peerId, QByteArray data);

private:
    struct TransferContext {
        QString transferId;
        QString peerId;
        QString localFilePath;
        QString fileName;
        quint64 fileSize{0};
        quint64 receivedBytes{0};
        bool isImage{false};
        bool accepted{false};
        bool rejected{false};
        QString downloadDirectoryOverride;
        flykylin::core::Message message;
    };

    void sendFileInternal(const QString& peerId, const QString& filePath, bool asImage);
    QString ensureDownloadDirectory(bool isImage) const;
    QString detectMimeType(const QString& filePath, bool asImage) const;

    communication::TcpConnectionManager* m_connectionManager;
    QString m_localUserId;
    QString m_downloadDirectory;
    QMap<QString, TransferContext> m_incomingTransfers;
    bool m_autoAcceptImages{true};
    bool m_autoAcceptFiles{true};
};

} // namespace services
} // namespace flykylin
