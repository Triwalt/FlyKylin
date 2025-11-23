#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFileInfo>

#include "core/services/FileTransferService.h"
#include "messages.pb.h"

using namespace flykylin;

namespace {

QByteArray makeFileRequest(const QString& transferId,
                           const QString& fromUser,
                           const QString& toUser,
                           const QString& fileName,
                           quint64 fileSize,
                           const QString& mimeType)
{
    protocol::FileTransferRequest req;
    req.set_transfer_id(transferId.toStdString());
    req.set_from_user_id(fromUser.toStdString());
    req.set_to_user_id(toUser.toStdString());
    req.set_file_name(fileName.toStdString());
    req.set_file_size(fileSize);
    req.set_file_hash(std::string());
    req.set_timestamp(0);
    req.set_mime_type(mimeType.toStdString());

    std::string payload;
    req.SerializeToString(&payload);

    protocol::TcpMessage tcpMsg;
    tcpMsg.set_protocol_version(1);
    tcpMsg.set_type(protocol::TcpMessage::FILE_REQUEST);
    tcpMsg.set_sequence(1);
    tcpMsg.set_payload(payload);
    tcpMsg.set_timestamp(0);

    QByteArray data(tcpMsg.ByteSizeLong(), Qt::Uninitialized);
    tcpMsg.SerializeToArray(data.data(), data.size());
    return data;
}

QByteArray makeFileChunk(const QString& transferId, const QByteArray& bytes)
{
    protocol::FileChunk chunk;
    chunk.set_transfer_id(transferId.toStdString());
    chunk.set_offset(0);
    chunk.set_data(bytes.constData(), bytes.size());
    chunk.set_chunk_size(static_cast<quint32>(bytes.size()));
    chunk.set_is_last(true);

    std::string payload;
    chunk.SerializeToString(&payload);

    protocol::TcpMessage tcpMsg;
    tcpMsg.set_protocol_version(1);
    tcpMsg.set_type(protocol::TcpMessage::FILE_CHUNK);
    tcpMsg.set_sequence(2);
    tcpMsg.set_payload(payload);
    tcpMsg.set_timestamp(0);

    QByteArray data(tcpMsg.ByteSizeLong(), Qt::Uninitialized);
    tcpMsg.SerializeToArray(data.data(), data.size());
    return data;
}

} // namespace

TEST(FileTransferServiceTest, AutoAcceptImageWritesFileAndEmitsMessage)
{
    services::FileTransferService service;

    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    service.setDownloadDirectory(tempDir.path());

    QSignalSpy createdSpy(&service, &services::FileTransferService::messageCreated);

    const QString transferId = QStringLiteral("transfer-1");
    const QString fromUser = QStringLiteral("user-a");
    const QString toUser = QStringLiteral("user-b");
    const QString fileName = QStringLiteral("test.png");
    const QByteArray content("dummy-image-data");

    QByteArray reqData = makeFileRequest(transferId, fromUser, toUser,
                                         fileName, static_cast<quint64>(content.size()),
                                         QStringLiteral("image/png"));
    service.handleIncomingTcpData(QStringLiteral("peer-1"), reqData);

    QByteArray chunkData = makeFileChunk(transferId, content);
    service.handleIncomingTcpData(QStringLiteral("peer-1"), chunkData);

    ASSERT_EQ(createdSpy.count(), 1);
    auto arguments = createdSpy.takeFirst();
    core::Message message = qvariant_cast<core::Message>(arguments.at(0));

    EXPECT_EQ(message.kind(), core::MessageKind::Image);
    EXPECT_EQ(message.attachmentName(), fileName);
    EXPECT_GT(message.attachmentLocalPath().size(), 0);

    QFileInfo savedInfo(message.attachmentLocalPath());
    EXPECT_TRUE(savedInfo.exists());
    EXPECT_EQ(savedInfo.size(), content.size());
}
