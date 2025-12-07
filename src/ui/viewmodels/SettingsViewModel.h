/**
 * @file SettingsViewModel.h
 * @brief View model for application/user settings exposed to QML
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace flykylin {
namespace ui {

class SettingsViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    Q_PROPERTY(QString avatarId READ avatarId WRITE setAvatarId NOTIFY avatarIdChanged)
    Q_PROPERTY(QString downloadDirectory READ downloadDirectory WRITE setDownloadDirectory NOTIFY downloadDirectoryChanged)
    Q_PROPERTY(QString chatHistoryDirectory READ chatHistoryDirectory WRITE setChatHistoryDirectory NOTIFY chatHistoryDirectoryChanged)
    Q_PROPERTY(QString localUserId READ localUserId CONSTANT)
    Q_PROPERTY(bool semanticSearchEnabled READ semanticSearchEnabled WRITE setSemanticSearchEnabled NOTIFY semanticSearchEnabledChanged)
    Q_PROPERTY(bool nsfwBlockOutgoing READ nsfwBlockOutgoing WRITE setNsfwBlockOutgoing NOTIFY nsfwBlockOutgoingChanged)
    Q_PROPERTY(bool nsfwBlockIncoming READ nsfwBlockIncoming WRITE setNsfwBlockIncoming NOTIFY nsfwBlockIncomingChanged)
    Q_PROPERTY(double nsfwThreshold READ nsfwThreshold WRITE setNsfwThreshold NOTIFY nsfwThresholdChanged)
    
    // Version info (read-only)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    Q_PROPERTY(QString buildDate READ buildDate CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
    Q_PROPERTY(QString platform READ platform CONSTANT)
    Q_PROPERTY(QStringList features READ features CONSTANT)

public:
    explicit SettingsViewModel(QObject* parent = nullptr);

    QString userName() const { return m_userName; }
    QString avatarId() const { return m_avatarId; }
    QString downloadDirectory() const { return m_downloadDirectory; }
    QString chatHistoryDirectory() const { return m_chatHistoryDirectory; }
    QString localUserId() const { return m_localUserId; }
    bool semanticSearchEnabled() const { return m_semanticSearchEnabled; }
    bool nsfwBlockOutgoing() const { return m_nsfwBlockOutgoing; }
    bool nsfwBlockIncoming() const { return m_nsfwBlockIncoming; }
    double nsfwThreshold() const { return m_nsfwThreshold; }
    
    // Version info getters
    QString appVersion() const;
    QString buildDate() const;
    QString qtVersion() const;
    QString platform() const;
    QStringList features() const;

public slots:
    void setUserName(const QString& userName);
    void setAvatarId(const QString& avatarId);
    void setDownloadDirectory(const QString& dir);
    void setChatHistoryDirectory(const QString& dir);
    void setSemanticSearchEnabled(bool enabled);
    void setNsfwBlockOutgoing(bool enabled);
    void setNsfwBlockIncoming(bool enabled);
    void setNsfwThreshold(double threshold);

public:
    Q_INVOKABLE void chooseDownloadDirectory();

signals:
    void userNameChanged();
    void avatarIdChanged();
    void downloadDirectoryChanged();
    void chatHistoryDirectoryChanged();
    void semanticSearchEnabledChanged();
    void nsfwBlockOutgoingChanged();
    void nsfwBlockIncomingChanged();
    void nsfwThresholdChanged();

private:
    void load();

    QString m_userName;
    QString m_avatarId;
    QString m_downloadDirectory;
    QString m_chatHistoryDirectory;
    QString m_localUserId;
    bool m_semanticSearchEnabled{false};
    bool m_nsfwBlockOutgoing{false};
    bool m_nsfwBlockIncoming{false};
    double m_nsfwThreshold{0.8};
};

} // namespace ui
} // namespace flykylin
