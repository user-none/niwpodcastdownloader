/*****************************************************************************
 *   Copyright (C) 2008 John Schember <john@nachtimwald.com>                 *
 *                                                                           *
 *   This file is part of niwpodcastdownloader.                              *
 *                                                                           *
 *   niwpodcastdownloader is free software: you can redistribute it and/or   *
 *   modify it under the terms of the GNU General Public License as          *
 *   published by the Free Software Foundation, either version 3 of the      *
 *   License, or (at your option) any later version.                         *
 *                                                                           *
 *   niwpodcastdownloader is distributed in the hope that it will be useful, *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with niwpodcastdownloader. If not, see                            *
 *   <http://www.gnu.org/licenses/>.                                         *
 *****************************************************************************/

#include <QDir>

#include "settingsmanager.h"

SettingsManager::SettingsManager()
{
    // Get the location the podcasts should be saved to.
    m_saveLocation = QDir::fromNativeSeparators(
        value("paths/podcast_save_location",
        QString("%1/podcasts")
        .arg(QDir::homePath())).toString());

    // Get the location of the file listing the podcast rss feeds.
    m_podcastsListFile = QDir::fromNativeSeparators(
        value("paths/podcast_xml_file",
        QString("%1/.niw-podcast-downloader/podcasts.xml")
        .arg(QDir::homePath())).toString());

    // Get the location of the file storing the db storing downloaded episodes.
    m_databaseFile = QDir::fromNativeSeparators(
        value("paths/podcast_episode_db",
        QString("%1/.niw-podcast-downloader/episodes.db")
        .arg(QDir::homePath())).toString());

    // Number of download threads to use.
    m_threadCount = value("advanced/thread_count", 1).toInt();
    if (m_threadCount < 1) {
        m_threadCount = 1;
    }

    // Download all new episodes or a specific number of new episodes.
    m_recentEpisodeCount = value("advanced/recent_episode_count", 0).toInt();
    if (m_recentEpisodeCount < 0) {
        m_recentEpisodeCount = 0;
    }

    // How much disk space is required to start a new download.
    m_minimumFreeDiskSpace = value("advanced/minimum_free_space", 0)
        .toLongLong();

    // Whether explict episodes should be filtered.
    m_filterExplicit = value("advanced/filter_explicit", false).toBool();

    // Whether the servers not modified response should be ignored.
    m_ignoreNotModified = value("network/ignore_not_modified", false).toBool();
}

void SettingsManager::writeDefaultConfig()
{
    setValue("paths/podcast_save_location", QString("%1/podcasts")
        .arg(QDir::homePath()));
    setValue("paths/podcast_xml_file",
        QString("%1/.niw-podcast-downloader/podcasts.xml")
        .arg(QDir::homePath()));
    setValue("paths/podcast_episode_db",
        QString("%1/.niw-podcast-downloader/episodes.db")
        .arg(QDir::homePath()));
    setValue("advanced/thread_count", 1);
    setValue("advanced/recent_episode_count", 0);
    setValue("advanced/minimum_free_space", 0);
    setValue("advanced/filter_explicit", 0);
    setValue("network/ignore_not_modified", 0);
}

QString SettingsManager::getSaveLocation()
{
    return m_saveLocation;
}

QString SettingsManager::getPodcastsFile()
{
    return m_podcastsListFile;
}

QString SettingsManager::getDatabaseFile()
{
    return m_databaseFile;
}

int SettingsManager::getThreadCount()
{
    return m_threadCount;
}

int SettingsManager::getRecentEpisodeCount()
{
    return m_recentEpisodeCount;
}

qlonglong SettingsManager::getMinimumFreeDiskSpace()
{
    return m_minimumFreeDiskSpace;
}

bool SettingsManager::getFilterExplicit()
{
    return m_filterExplicit;
}

bool SettingsManager::getIgnoreNotModified()
{
    return m_ignoreNotModified;
}

void SettingsManager::setSaveLocation(const QString &location)
{
    m_saveLocation = location;
}

void SettingsManager::setPodcastsFile(const QString &file)
{
    m_podcastsListFile = file;
}

void SettingsManager::setDownloadedEpisodeListFile(const QString &file)
{
    m_databaseFile = file;
}

void SettingsManager::setThreadCount(int count)
{
    m_threadCount = qMax(count, 1);
}

void SettingsManager::setRecentEpisodeCount(int count)
{
    m_recentEpisodeCount = qMax(count, 0);
}

void SettingsManager::setMinimumFreeDiskSpace(qlonglong size)
{
    m_minimumFreeDiskSpace = qMax(size,  Q_INT64_C(-1));
}

void SettingsManager::setFilterExplicit(bool filterExplicit)
{
    m_filterExplicit = filterExplicit;
}

void SettingsManager::setIgnoreNotModified(bool ignore)
{
    m_ignoreNotModified = ignore;
}
