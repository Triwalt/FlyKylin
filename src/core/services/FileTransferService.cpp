#include "FileTransferService.h"

#include "../config/UserProfile.h"
#include "../ai/NSFWDetector.h"
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

bool nsfwBlockOutgoing()
{
    QSettings settings("FlyKylin", "FlyKylin");
    return settings.value("nsfw/blockOutgoing", false).toBool();
}

bool nsfwBlockIncoming()
{
    QSettings settings("FlyKylin", "FlyKylin");
    return settings.value("nsfw/blockIncoming", false).toBool();
}

double nsfwThreshold()
{
    QSettings settings("FlyKylin", "FlyKylin");
    double value = settings.value("nsfw/threshold", 0.8).toDouble();
    if (value < 0.0) {
        value = 0.0;
    } else if (value > 1.0) {
        value = 1.0;
    }
    return value;
}
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
    sendFileInternal(peerId,
                     filePath,
                     true,   // asImage
                     false,  // isGroup
                     QString(),
                     QString());
}

void FileTransferService::sendFile(const QString& peerId, const QString& filePath)
{
    sendFileInternal(peerId,
                     filePath,
                     false,  // asImage
                     false,  // isGroup
                     QString(),
                     QString());
}

void FileTransferService::sendImage(const QString& peerId,
                                    const QString& filePath,
                                    bool isGroup,
                                    const QString& groupId,
                                    const QString& logicalMessageId)
{
    sendFileInternal(peerId,
                     filePath,
                     true,  // asImage
                     isGroup,
                     groupId,
                     logicalMessageId);
}

void FileTransferService::sendFile(const QString& peerId,
                                   const QString& filePath,
                                   bool isGroup,
                                   const QString& groupId,
                                   const QString& logicalMessageId)
{
    sendFileInternal(peerId,
                     filePath,
                     false,  // asImage
                     isGroup,
                     groupId,
                     logicalMessageId);
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

void FileTransferService::sendFileInternal(const QString& peerId,
                                           const QString& filePath,
                                           bool asImage,
                                           bool isGroup,
                                           const QString& groupId,
                                           const QString& logicalMessageId)
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

    float nsfwProb = 0.0f;
    bool nsfwProbValid = false;
    bool nsfwChecked = false;
    bool nsfwPassedFlag = false;

    if (asImage) {
        auto* detector = ai::NSFWDetector::instance();
        if (detector && detector->isAvailable()) {
            const auto prob = detector->predictNsfwProbability(filePath);
            if (prob.has_value()) {
                nsfwProb = *prob;
                nsfwProbValid = true;
                qInfo() << "[FileTransferService] NSFW probability for outgoing image" << filePath
                        << "=" << nsfwProb;
            } else {
                qWarning() << "[FileTransferService] NSFW detection failed for outgoing image"
                           << filePath;
            }
        }
    }

    if (asImage && nsfwProbValid && nsfwBlockOutgoing()) {
        const double threshold = nsfwThreshold();
        const bool blocked = nsfwProb >= static_cast<float>(threshold);

        if (blocked) {
            QString infoText =
                QStringLiteral("[NSFW] 发送图片检测: 阻断 (p=%1, 阈值=%2)")
                    .arg(static_cast<double>(nsfwProb), 0, 'f', 3)
                    .arg(threshold, 0, 'f', 2);

            core::Message infoMessage;
            infoMessage.setId(core::Message::generateMessageId());
            infoMessage.setFromUserId(m_localUserId);
            infoMessage.setToUserId(peerId);
            infoMessage.setTimestamp(QDateTime::currentDateTime());
            infoMessage.setStatus(core::MessageStatus::Delivered);
            infoMessage.setKind(core::MessageKind::Text);
            infoMessage.setContent(infoText);
            if (isGroup && !groupId.isEmpty()) {
                infoMessage.setIsGroup(true);
                infoMessage.setGroupId(groupId);
            }

            emit messageCreated(infoMessage);

            emit transferFailed(QString(),
                                QStringLiteral("NSFW policy blocked outgoing image"));
            return;
        } else {
            nsfwChecked = true;
            nsfwPassedFlag = true;
        }
    }

    QString mimeType = detectMimeType(filePath, asImage);
    QString transferId = logicalMessageId.isEmpty()
            ? core::Message::generateMessageId()
            : logicalMessageId;

    flykylin::protocol::FileTransferRequest req;
    req.set_transfer_id(transferId.toStdString());
    req.set_from_user_id(m_localUserId.toStdString());
    req.set_to_user_id(peerId.toStdString());
    req.set_file_name(info.fileName().toStdString());
    req.set_file_size(fileSize);
    req.set_file_hash(std::string());
    req.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    req.set_mime_type(mimeType.toStdString());
    req.set_is_group(isGroup);
    if (isGroup && !groupId.isEmpty()) {
        req.set_group_id(groupId.toStdString());
    }

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
    if (isGroup && !groupId.isEmpty()) {
        message.setIsGroup(true);
        message.setGroupId(groupId);
    }

    if (asImage && nsfwChecked) {
        message.setNsfwChecked(true);
        message.setNsfwPassed(nsfwPassedFlag);
    }

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

        const QString mimeType = QString::fromStdString(req.mime_type());
        ctx.isImage = mimeType.startsWith(QStringLiteral("image/"));
        ctx.accepted = ctx.isImage ? m_autoAcceptImages : m_autoAcceptFiles;
        ctx.isGroup = req.is_group();
        if (ctx.isGroup) {
            ctx.groupId = QString::fromStdString(req.group_id());
        }

        core::Message message;
        message.setId(transferId);
        message.setFromUserId(QString::fromStdString(req.from_user_id()));
        message.setToUserId(QString::fromStdString(req.to_user_id()));
        message.setTimestamp(QDateTime::fromMSecsSinceEpoch(req.timestamp()));
        message.setStatus(core::MessageStatus::Sending);
        message.setKind(ctx.isImage ? core::MessageKind::Image : core::MessageKind::File);
        message.setAttachmentName(ctx.fileName);
        message.setAttachmentSize(ctx.fileSize);
        message.setMimeType(mimeType);
        message.setContent(ctx.fileName);
        if (ctx.isGroup && !ctx.groupId.isEmpty()) {
            message.setIsGroup(true);
            message.setGroupId(ctx.groupId);
        }

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

            float nsfwProbIncoming = 0.0f;
            bool nsfwProbIncomingValid = false;
            bool nsfwCheckedIncoming = false;
            bool nsfwPassedIncoming = false;

            if (ctx.isImage) {
                auto* detector = ai::NSFWDetector::instance();
                if (detector && detector->isAvailable()) {
                    const auto prob = detector->predictNsfwProbability(ctx.localFilePath);
                    if (prob.has_value()) {
                        nsfwProbIncoming = *prob;
                        nsfwProbIncomingValid = true;
                        qInfo() << "[FileTransferService] NSFW probability for received image"
                                << ctx.localFilePath << "=" << nsfwProbIncoming;
                    } else {
                        qWarning() << "[FileTransferService] NSFW detection failed for received image"
                                   << ctx.localFilePath;
                    }
                }
            }

            if (ctx.isImage && nsfwProbIncomingValid && nsfwBlockIncoming()) {
                const double threshold = nsfwThreshold();
                const bool blocked = nsfwProbIncoming >= static_cast<float>(threshold);

                if (blocked) {
                    QString infoText =
                        QStringLiteral("[NSFW] 接收来自 %1 的图片检测: 阻断 (p=%2, 阈值=%3)")
                            .arg(ctx.peerId)
                            .arg(static_cast<double>(nsfwProbIncoming), 0, 'f', 3)
                            .arg(threshold, 0, 'f', 2);

                    core::Message infoMessage;
                    infoMessage.setId(core::Message::generateMessageId());
                    infoMessage.setFromUserId(m_localUserId);
                    infoMessage.setToUserId(ctx.peerId);
                    infoMessage.setTimestamp(QDateTime::currentDateTime());
                    infoMessage.setStatus(core::MessageStatus::Delivered);
                    infoMessage.setKind(core::MessageKind::Text);
                    infoMessage.setContent(infoText);
                    if (ctx.isGroup && !ctx.groupId.isEmpty()) {
                        infoMessage.setIsGroup(true);
                        infoMessage.setGroupId(ctx.groupId);
                    }

                    emit messageCreated(infoMessage);

                    QFile::remove(ctx.localFilePath);
                    m_incomingTransfers.remove(transferId);
                    emit transferFailed(transferId,
                                        QStringLiteral("NSFW policy blocked incoming image"));
                    return;
                } else {
                    nsfwCheckedIncoming = true;
                    nsfwPassedIncoming = true;
                }
            }

            core::Message completedMessage = ctx.message;
            if (ctx.isImage && nsfwCheckedIncoming) {
                completedMessage.setNsfwChecked(true);
                completedMessage.setNsfwPassed(nsfwPassedIncoming);
            }
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
