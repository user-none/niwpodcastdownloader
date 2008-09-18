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
    m_saveLocation = QDir::fromNativeSeparators(value(
        "paths/podcast_save_location", QDir::homePath() + "/podcasts")
        .toString());

    // Get the location of the file listing the podcast rss feeds.
    m_podcastsListFile = value("paths/podcast_xml_file",
        QDir::homePath() + "/.niw-podcast-downloader/podcasts.xml").toString();

    // Get the location of the file storing the db storing downloaded episodes.
    m_downloadedEpisodeListFile = value("paths/podcast_episode_db",
        QDir::homePath() + "/.niw-podcast-downloader/episodes.db").toString();

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
}

void SettingsManager::writeDefaultConfig()
{
    setValue("paths/podcast_save_location", QDir::homePath() + "/podcasts");
    setValue("paths/podcast_xml_file",
        QDir::homePath() + "/.niw-podcast-downloader/podcasts.xml");
    setValue("paths/podcast_episode_db",
        QDir::homePath() + "/.niw-podcast-downloader/episodes.db");
    setValue("advanced/thread_count", 1);
    setValue("advanced/recent_episode_count", 0);
    setValue("advanced/minimum_free_space", 0);
}

QString SettingsManager::getSaveLocation()
{
    return m_saveLocation;
}

QString SettingsManager::getPodcastsFile()
{
    return m_podcastsListFile;
}

QString SettingsManager::getDownloadedEpisodeListFile()
{
    return m_downloadedEpisodeListFile;
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
    m_downloadedEpisodeListFile = file;
}

void SettingsManager::setThreadCount(int count)
{
    if (count < 1) {
        m_threadCount = 1;
    }
    else {
        m_threadCount = count;
    }
}

void SettingsManager::setRecentEpisodeCount(int count)
{
    if (count < 0) {
        m_recentEpisodeCount = 0;
    }
    else {
        m_recentEpisodeCount = count;
    }
}

void SettingsManager::setMinimumFreeDiskSpace(qlonglong size)
{
    if (size < -1) {
        m_minimumFreeDiskSpace = -1;
    }
    else {
        m_minimumFreeDiskSpace = size;
    }
}
