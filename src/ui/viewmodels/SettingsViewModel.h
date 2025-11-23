/**
 * @file SettingsViewModel.h
 * @brief View model for application/user settings exposed to QML
 */

#pragma once

#include <QObject>
#include <QString>

namespace flykylin {
namespace ui {

class SettingsViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    Q_PROPERTY(QString avatarId READ avatarId WRITE setAvatarId NOTIFY avatarIdChanged)
    Q_PROPERTY(QString downloadDirectory READ downloadDirectory WRITE setDownloadDirectory NOTIFY downloadDirectoryChanged)
    Q_PROPERTY(QString chatHistoryDirectory READ chatHistoryDirectory WRITE setChatHistoryDirectory NOTIFY chatHistoryDirectoryChanged)
    Q_PROPERTY(QString localUserId READ localUserId CONSTANT)

public:
    explicit SettingsViewModel(QObject* parent = nullptr);

    QString userName() const { return m_userName; }
    QString avatarId() const { return m_avatarId; }
    QString downloadDirectory() const { return m_downloadDirectory; }
    QString chatHistoryDirectory() const { return m_chatHistoryDirectory; }
    QString localUserId() const { return m_localUserId; }

public slots:
    void setUserName(const QString& userName);
    void setAvatarId(const QString& avatarId);
    void setDownloadDirectory(const QString& dir);
    void setChatHistoryDirectory(const QString& dir);

public:
    Q_INVOKABLE void chooseDownloadDirectory();

signals:
    void userNameChanged();
    void avatarIdChanged();
    void downloadDirectoryChanged();
    void chatHistoryDirectoryChanged();

private:
    void load();

    QString m_userName;
    QString m_avatarId;
    QString m_downloadDirectory;
    QString m_chatHistoryDirectory;
    QString m_localUserId;
};

} // namespace ui
} // namespace flykylin
