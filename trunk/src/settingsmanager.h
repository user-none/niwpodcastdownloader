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

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QSettings>

/**
 * Reads and caches the user settings for the application.
 *
 * Using a setter will not change the config file. It only changes the cached
 * value for the current instance.
 */
class SettingsManager : private QSettings
{
    public:
        SettingsManager();

        /**
         * Write the default config to disk.
         *
         * This will over write any existing settings.
         */
        void writeDefaultConfig();

        /**
         * Gets the location that podcasts should be saved in.
         *
         * @return The directory that the podcasts should be saved to.
         */
        QString getSaveLocation();
        /**
         * The file that lists the podcast rss feeds to check.
         *
         * @return The podcast rss feed listing file including the full path.
         */
        QString getPodcastsFile();
        /**
         * The file that lists the previously downloaded episodes for all
         * podcasts and the last modified date for podcast rss feeds.
         *
         * @return The database file including the full path.
         */
        QString getDatabaseFile();
        /**
         * The number of download threads that should be used.
         *
         * @return The number of threads. Will always be >= 1.
         */
        int getThreadCount();
        /**
         * The number of recent not previously downloaded episodes to download.
         *
         * @return 0 for all new episodes. Otherwise, the maximum number of
         * episodes to download.
         */
        int getRecentEpisodeCount();
        /**
         * The minimum amount of free disk space that must be available to
         * start a download.
         *
         * @return The amount of disk space in KB. Anything less than 0 is
         * ignored.
         */
        qlonglong getMinimumFreeDiskSpace();
        /**
         * Should explicit episodes be ignored.
         *
         * @return True if explicit episodes should be ignored.
         */
        bool getFilterExplicit();

        /**
         * Sets the location that podcasts should be saved in.
         *
         * @param location The directory that the podcasts should be saved to.
         */
        void setSaveLocation(const QString &location);
        /**
         * The file that lists the podcast rss feeds to check.
         *
         * @param file The podcast rss feed listing file including the full path.
         */
        void setPodcastsFile(const QString &file);
        /**
         * The file that lists the previously downloaded episodes for all
         * podcasts.
         *
         * @param file The episode listing file including the full path.
         */
        void setDownloadedEpisodeListFile(const QString &file);
        /**
         * The number of download threads that should be used.
         *
         * @param count The number of threads.
         */
        void setThreadCount(int count);
        /**
         * The number of recent not previously downloaded episodes to download.
         *
         * @param count 0 for all new episodes. Otherwise, the maximum number of
         * episodes to download.
         */
        void setRecentEpisodeCount(int count);
        /**
         * The minimum amount of free disk space that must be available to
         * start a download.
         *
         * @param size The amount of disk space in KB.
         */
        void setMinimumFreeDiskSpace(qlonglong size);
        /**
         * Should explicit episodes be ignored.
         *
         * @param filterExplicit True if explicit episodes should be ignored.
         */
        void setFilterExplicit(bool filterExplicit);

    private:
        /**
         * The directory to save in.
         */
        QString m_saveLocation;
        /**
         * The xml file name with path that has a list of podcasts to download.
         */
        QString m_podcastsListFile;
        /**
         * The sqlite db file name with path that is used for checking and
         * setting downloaded episodes as downloaded. Also checks and sets
         * The last modified date for podcast rss feeds.
         */
        QString m_databaseFile;
        /**
         * The maximum number of simultaneous downloads to run at one time.
         */
        int m_threadCount;
        /**
         * The maximum number of recent episodes to download.
         */
        int m_recentEpisodeCount;
        /**
         * The minimum amount of free disk space required to start a new
         * download.
         */
        qlonglong m_minimumFreeDiskSpace;
        /**
         * Whether explicit episodes should be ignored;
         */
        bool m_filterExplicit;
};

#endif /* SETTINGSMANAGER_H */
