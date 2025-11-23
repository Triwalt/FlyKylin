/**
 * @file SettingsViewModel.cpp
 * @brief Implementation of SettingsViewModel for QML settings page
 */

#include "SettingsViewModel.h"

#include "core/config/UserProfile.h"

#include <QSettings>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

namespace flykylin {
namespace ui {

SettingsViewModel::SettingsViewModel(QObject* parent)
    : QObject(parent)
{
    load();
}

void SettingsViewModel::load()
{
    // Load from UserProfile
    flykylin::core::UserProfile& profile = flykylin::core::UserProfile::instance();
    m_userName = profile.userName();
    m_avatarId = profile.avatarPath();
    m_localUserId = profile.userId();

    // Load from QSettings
    QSettings settings("FlyKylin", "FlyKylin");
    m_downloadDirectory = settings.value("paths/downloadDirectory").toString();
    m_chatHistoryDirectory = settings.value("paths/chatHistoryDirectory").toString();
}

void SettingsViewModel::setUserName(const QString& userName)
{
    if (m_userName == userName) {
        return;
    }

    m_userName = userName;
    flykylin::core::UserProfile::instance().setUserName(userName);
    emit userNameChanged();
}

void SettingsViewModel::setAvatarId(const QString& avatarId)
{
    if (m_avatarId == avatarId) {
        return;
    }

    m_avatarId = avatarId;
    flykylin::core::UserProfile::instance().setAvatarPath(avatarId);
    emit avatarIdChanged();
}

void SettingsViewModel::setDownloadDirectory(const QString& dir)
{
    if (m_downloadDirectory == dir) {
        return;
    }

    m_downloadDirectory = dir;

    QSettings settings("FlyKylin", "FlyKylin");
    settings.setValue("paths/downloadDirectory", m_downloadDirectory);
    settings.sync();

    emit downloadDirectoryChanged();
}

void SettingsViewModel::setChatHistoryDirectory(const QString& dir)
{
    if (m_chatHistoryDirectory == dir) {
        return;
    }

    m_chatHistoryDirectory = dir;

    QSettings settings("FlyKylin", "FlyKylin");
    settings.setValue("paths/chatHistoryDirectory", m_chatHistoryDirectory);
    settings.sync();

    emit chatHistoryDirectoryChanged();
}

void SettingsViewModel::chooseDownloadDirectory()
{
    QString currentDir = m_downloadDirectory;

    if (currentDir.isEmpty()) {
        QString baseDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        if (baseDir.isEmpty()) {
            baseDir = QDir::homePath();
        }
        currentDir = baseDir;
    }

    QString selectedDir = QFileDialog::getExistingDirectory(nullptr,
                                                            QObject::tr("选择下载目录"),
                                                            currentDir);
    if (selectedDir.isEmpty()) {
        return;
    }

    setDownloadDirectory(selectedDir);
    qInfo() << "[SettingsViewModel] Download directory set to" << selectedDir;
}

} // namespace ui
} // namespace flykylin
