#include "FileTransferService.h"

#include "../config/UserProfile.h"
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <string>
#include "messages.pb.h"

namespace {
constexpr quint64 kMaxFileSizeBytes = 200ull * 1024ull * 1024ull;
constexpr qint64 kChunkSizeBytes = 1024 * 1024; // 1MB
}

namespace flykylin {
namespace services {

FileTransferService::FileTransferService(QObject* parent)
    : QObject(parent)
    , m_connectionManager(communication::TcpConnectionManager::instance())
{
    m_localUserId = core::UserProfile::instance().userId();

    connect(m_connectionManager, &communication::TcpConnectionManager::messageReceived,
            this, &FileTransferService::onTcpMessageReceived);
}

FileTransferService::~FileTransferService() = default;

void FileTransferService::sendImage(const QString& peerId, const QString& filePath)
{
    sendFileInternal(peerId, filePath, true);
}

void FileTransferService::sendFile(const QString& peerId, const QString& filePath)
{
    sendFileInternal(peerId, filePath, false);
}

void FileTransferService::setDownloadDirectory(const QString& path)
{
    m_downloadDirectory = path;

    QSettings settings("FlyKylin", "FlyKylin");
    settings.setValue("paths/downloadDirectory", m_downloadDirectory);
    settings.sync();
}

QString FileTransferService::downloadDirectory() const
{
    return m_downloadDirectory;
}

void FileTransferService::sendFileInternal(const QString& peerId, const QString& filePath, bool asImage)
{
    QFileInfo info(filePath);
    if (!info.exists() || !info.isFile()) {
        emit transferFailed(QString(), QStringLiteral("File not found"));
        return;
    }

    quint64 fileSize = static_cast<quint64>(info.size());
    if (fileSize > kMaxFileSizeBytes) {
        emit transferFailed(QString(), QStringLiteral("File is too large (max 200MB)"));
        return;
    }

    QString mimeType = detectMimeType(filePath, asImage);
    QString transferId = core::Message::generateMessageId();

    flykylin::protocol::FileTransferRequest req;
    req.set_transfer_id(transferId.toStdString());
    req.set_from_user_id(m_localUserId.toStdString());
    req.set_to_user_id(peerId.toStdString());
    req.set_file_name(info.fileName().toStdString());
    req.set_file_size(fileSize);
    req.set_file_hash(std::string());
    req.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    req.set_mime_type(mimeType.toStdString());

    std::string payload;
    if (!req.SerializeToString(&payload)) {
        emit transferFailed(transferId, QStringLiteral("Failed to serialize FileTransferRequest"));
        return;
    }

    flykylin::protocol::TcpMessage tcpReq;
    tcpReq.set_protocol_version(1);
    tcpReq.set_type(flykylin::protocol::TcpMessage::FILE_REQUEST);
    tcpReq.set_sequence(0);
    tcpReq.set_payload(payload);
    tcpReq.set_timestamp(QDateTime::currentMSecsSinceEpoch());

    QByteArray reqData(tcpReq.ByteSizeLong(), Qt::Uninitialized);
    if (!tcpReq.SerializeToArray(reqData.data(), reqData.size())) {
        emit transferFailed(transferId, QStringLiteral("Failed to serialize TcpMessage (FILE_REQUEST)"));
        return;
    }

    m_connectionManager->sendMessage(peerId, reqData,
                                     communication::MessageQueue::Priority::Normal);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit transferFailed(transferId, QStringLiteral("Failed to open file"));
        return;
    }

    quint64 offset = 0;

    while (true) {
        QByteArray fileData = file.read(kChunkSizeBytes);
        if (fileData.isEmpty()) {
            if (file.error() != QFile::NoError) {
                emit transferFailed(transferId, QStringLiteral("Failed to read from file"));
                file.close();
                return;
            }
            break;
        }

        flykylin::protocol::FileChunk chunk;
        chunk.set_transfer_id(transferId.toStdString());
        chunk.set_offset(static_cast<quint64>(offset));
        chunk.set_data(fileData.constData(), static_cast<int>(fileData.size()));
        chunk.set_chunk_size(static_cast<quint32>(fileData.size()));
        chunk.set_is_last(file.atEnd());

        std::string chunkPayload;
        if (!chunk.SerializeToString(&chunkPayload)) {
            emit transferFailed(transferId, QStringLiteral("Failed to serialize FileChunk"));
            file.close();
            return;
        }

        flykylin::protocol::TcpMessage tcpChunk;
        tcpChunk.set_protocol_version(1);
        tcpChunk.set_type(flykylin::protocol::TcpMessage::FILE_CHUNK);
        tcpChunk.set_sequence(0);
        tcpChunk.set_payload(chunkPayload);
        tcpChunk.set_timestamp(QDateTime::currentMSecsSinceEpoch());

        QByteArray chunkData(tcpChunk.ByteSizeLong(), Qt::Uninitialized);
        if (!tcpChunk.SerializeToArray(chunkData.data(), chunkData.size())) {
            emit transferFailed(transferId, QStringLiteral("Failed to serialize TcpMessage (FILE_CHUNK)"));
            file.close();
            return;
        }

        m_connectionManager->sendMessage(peerId, chunkData,
                                         communication::MessageQueue::Priority::Normal);

        offset += static_cast<quint64>(fileData.size());
    }

    file.close();

    core::Message message;
    message.setId(transferId);
    message.setFromUserId(m_localUserId);
    message.setToUserId(peerId);
    message.setTimestamp(QDateTime::currentDateTime());
    message.setStatus(core::MessageStatus::Sent);
    message.setKind(asImage ? core::MessageKind::Image : core::MessageKind::File);
    message.setAttachmentLocalPath(filePath);
    message.setAttachmentName(info.fileName());
    message.setAttachmentSize(static_cast<quint64>(info.size()));
    message.setMimeType(mimeType);
    message.setContent(info.fileName());

    emit messageCreated(message);
    emit transferCompleted(transferId, message);
}

QString FileTransferService::ensureDownloadDirectory(bool isImage) const
{
    QString baseDir = m_downloadDirectory;

    if (baseDir.isEmpty()) {
        QSettings settings("FlyKylin", "FlyKylin");
        baseDir = settings.value("paths/downloadDirectory").toString();
    }

    if (baseDir.isEmpty()) {
        QStandardPaths::StandardLocation loc = isImage
                ? QStandardPaths::PicturesLocation
                : QStandardPaths::DownloadLocation;

        baseDir = QStandardPaths::writableLocation(loc);
        if (baseDir.isEmpty()) {
            baseDir = QDir::homePath() + QStringLiteral("/FlyKylinDownloads");
        } else {
            baseDir += QStringLiteral("/FlyKylin");
        }
    }

    QDir dir(baseDir);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }

    return dir.absolutePath();
}

QString FileTransferService::detectMimeType(const QString& filePath, bool asImage) const
{
    QFileInfo info(filePath);
    QString suffix = info.suffix().toLower();

    if (asImage) {
        if (suffix == QStringLiteral("png")) {
            return QStringLiteral("image/png");
        }
        if (suffix == QStringLiteral("jpg") || suffix == QStringLiteral("jpeg")) {
            return QStringLiteral("image/jpeg");
        }
        if (suffix == QStringLiteral("gif")) {
            return QStringLiteral("image/gif");
        }
        return QStringLiteral("image/*");
    }

    return QStringLiteral("application/octet-stream");
}

void FileTransferService::onTcpMessageReceived(QString peerId, QByteArray data)
{
    handleIncomingTcpData(peerId, data);
}

void FileTransferService::handleIncomingTcpData(const QString& peerId, const QByteArray& data)
{
    flykylin::protocol::TcpMessage tcpMsg;
    if (!tcpMsg.ParseFromArray(data.data(), data.size())) {
        return;
    }

    using MessageType = flykylin::protocol::TcpMessage::MessageType;
    MessageType type = tcpMsg.type();

    if (type != flykylin::protocol::TcpMessage::FILE_REQUEST &&
        type != flykylin::protocol::TcpMessage::FILE_CHUNK) {
        return;
    }

    if (type == flykylin::protocol::TcpMessage::FILE_REQUEST) {
        flykylin::protocol::FileTransferRequest req;
        if (!req.ParseFromString(tcpMsg.payload())) {
            return;
        }

        QString transferId = QString::fromStdString(req.transfer_id());

        TransferContext ctx;
        ctx.transferId = transferId;
        ctx.peerId = peerId;
        ctx.fileName = QString::fromStdString(req.file_name());
        ctx.fileSize = req.file_size();
        ctx.isImage = QString::fromStdString(req.mime_type()).startsWith(QStringLiteral("image/"));
        ctx.accepted = ctx.isImage ? m_autoAcceptImages : m_autoAcceptFiles;

        core::Message message;
        message.setId(transferId);
        message.setFromUserId(QString::fromStdString(req.from_user_id()));
        message.setToUserId(QString::fromStdString(req.to_user_id()));
        message.setTimestamp(QDateTime::fromMSecsSinceEpoch(req.timestamp()));
        message.setStatus(core::MessageStatus::Sending);
        message.setKind(ctx.isImage ? core::MessageKind::Image : core::MessageKind::File);
        message.setAttachmentName(ctx.fileName);
        message.setAttachmentSize(ctx.fileSize);
        message.setMimeType(QString::fromStdString(req.mime_type()));
        message.setContent(ctx.fileName);

        ctx.message = message;
        m_incomingTransfers.insert(transferId, ctx);
        emit incomingTransferRequested(transferId, peerId, message);
        return;
    }

    if (type == flykylin::protocol::TcpMessage::FILE_CHUNK) {
        flykylin::protocol::FileChunk chunk;
        if (!chunk.ParseFromString(tcpMsg.payload())) {
            return;
        }

        QString transferId = QString::fromStdString(chunk.transfer_id());
        auto it = m_incomingTransfers.find(transferId);
        if (it == m_incomingTransfers.end()) {
            return;
        }

        TransferContext& ctx = it.value();

        if (ctx.rejected) {
            m_incomingTransfers.remove(transferId);
            return;
        }

        if (!ctx.accepted) {
            return;
        }

        QString baseDir = ctx.downloadDirectoryOverride.isEmpty()
                ? ensureDownloadDirectory(ctx.isImage)
                : ctx.downloadDirectoryOverride;
        QString filePath = baseDir + QDir::separator() + ctx.fileName;

        QFile outFile(filePath);
        QIODevice::OpenMode openMode = QIODevice::WriteOnly;
        if (ctx.receivedBytes > 0) {
            openMode |= QIODevice::Append;
        }

        if (!outFile.open(openMode)) {
            emit transferFailed(transferId, QStringLiteral("Failed to open output file"));
            m_incomingTransfers.remove(transferId);
            return;
        }

        const std::string& dataRef = chunk.data();
        qint64 written = outFile.write(dataRef.data(), static_cast<qint64>(dataRef.size()));
        outFile.close();
        if (written != static_cast<qint64>(dataRef.size())) {
            emit transferFailed(transferId, QStringLiteral("Failed to write output file"));
            m_incomingTransfers.remove(transferId);
            return;
        }

        ctx.localFilePath = filePath;
        ctx.receivedBytes += static_cast<quint64>(dataRef.size());
        ctx.message.setAttachmentLocalPath(filePath);

        if (chunk.is_last()) {
            ctx.message.setStatus(core::MessageStatus::Delivered);
            ctx.message.setAttachmentSize(ctx.receivedBytes);

            core::Message completedMessage = ctx.message;
            m_incomingTransfers.remove(transferId);

            emit messageCreated(completedMessage);
            emit transferCompleted(transferId, completedMessage);
        }
    }
}

void FileTransferService::acceptTransfer(const QString& transferId, const QString& targetDirectory)
{
    if (!m_incomingTransfers.contains(transferId)) {
        emit transferFailed(transferId, QStringLiteral("Unknown transfer"));
        return;
    }

    TransferContext& ctx = m_incomingTransfers[transferId];
    ctx.accepted = true;
    if (!targetDirectory.isEmpty()) {
        ctx.downloadDirectoryOverride = targetDirectory;
    }
}

void FileTransferService::rejectTransfer(const QString& transferId, const QString& reason)
{
    if (!m_incomingTransfers.contains(transferId)) {
        emit transferFailed(transferId, QStringLiteral("Unknown transfer"));
        return;
    }

    TransferContext& ctx = m_incomingTransfers[transferId];
    ctx.rejected = true;

    QString finalReason = reason.isEmpty() ? QStringLiteral("Rejected by receiver") : reason;
    emit transferFailed(transferId, finalReason);
}

} // namespace services
} // namespace flykylin
