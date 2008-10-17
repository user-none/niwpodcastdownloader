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

#ifndef CLIENT_H
#define CLIENT_H

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QQueue>
#include <QTextStream>

#include "database.h"
#include "podcast.h"
#include "podcastepisode.h"
#include "settingsmanager.h"

/**
 * The download client.
 *
 * Call run to start the client.
 */
class Client : public QObject
{
    Q_OBJECT

    public:
        Client();
        ~Client();

    public slots:
        /**
         * The main application instance.
         */
        void run();

    private slots:
        /**
         * Start the next download.
         *
         * If there are no more files to download and there are no files
         * currently downloading the application will exit.
         */
        void downloadNext();
        /**
         * Writes the error associated with the download to stderr.
         *
         * Will start the next download.
         *
         * @see downloadNext
         */
        void downloadError(DownloadItem *item, QString errorString);
        /**
         * Writes the download message to stderr.
         *
         * Optionally exits the application if the error is fatal.
         *
         * @param error The error string.
         * @param fatal True if this is a fatal error and the application
         * should exit.
         */
        void error(const QString &error, bool fatal=false);

        /**
         * Download a podcasts rss feed.
         *
         * This slot should only be linked to a singal sending a Podcast
         * object.
         *
         * Either a podcast will be taken from m_podcastRSSQueue or one can
         * be given. If one is given it is because of a 301 or 301 content
         * moved repose from the podcast.
         *
         * @param item An optional Podcast to use. If one is not given
         * a Podcast will be taken from m_podcastRSSQueue and used.
         * @param url An optional url to download the feed from. If this is not
         * given the podcast url will be used.
         */
        void startRSSDownload(DownloadItem *item=0, QUrl url=QUrl());
        /**
         * Generate a list of episodes to download.
         *
         * This slot should only be linked to a singal sending a Podcast
         * object.
         *
         * Checks for new episodes and on user settings adds them to
         * m_podcastDownloadQueue. If the podcast is set to init mode all new
         * episodes will be marked as downloaded.
         *
         * @param item The Podcast to use for generating a list of episodes.
         */
        void episodesReady(DownloadItem *item);

        /**
         * Download a podcast episode.
         *
         * This slot should only be linked to a singal sending a PodcastEpisode
         * object.
         *
         * Either an episode will be taken from m_podcastDownloadQueue or one
         * can be given. If one is given it is because of a 301 or 302 content
         * moved response from the episode download.
         *
         * @param item An optional PodcastEpisode to use. If one is not given
         * a PodcastEpisode will be taken from m_podcastDownloadQueue.
         * @param url An optional url to download the episode from. If this is
         * not given the PodcastEpisode url will be used.
         */
        void startEpisodeDownload(DownloadItem *item=0, QUrl url=QUrl());
        /**
         * The podcast episode has been successfully downloaded.
         *
         * This slot should only be linked to a singal sending a PodcastEpisode
         * object.
         *
         * Sets the episode as downloaded and starts the next download.
         *
         * @param item The PodcastEpisode that has been downloaded.
         */
        void episodeDownloaded(DownloadItem *item);

        /**
         * The download item has not been modified since the last time it was
         * download.
         *
         * This slot is best used when linked to a Podcast object and
         * downloading its rss feed. This will keep the client from downloading
         * extra data and processing data when there are no new items.
         *
         * @param item The download item returning its status as not modified.
         */
        void downloadItemNotModified(DownloadItem *item);

    private:
        /**
         * Open the database file and make it ready for use.
         */
        void loadDatabase();
        /**
         * Load the podcasts from the listings file into the rss queue so their
         * rss feeds can be checked for new episodes.
         */
        void loadPodcasts();
        /**
         * Gets a network request object and populates it with necessary
         * headers.
         *
         * @return A network requeset.
         */
        QNetworkRequest getNetworkRequest();
        /**
         * Check and write message for verbose mode.
         */
        void verbose(const QString &message);
        /**
         * Parse the command line arguments.
         */
        void parseOptions();

        /**
         * Run the application in init mode.
         *
         * Init mode does not download anything. It just marks all episodes
         * as downloaded.
         */
        bool m_initMode;
        /**
         * Run the application in verbose mode.
         *
         * Output what the application is doing. Useful for testing.
         */
        bool m_verboseMode;
        /**
         * Ignore checks for the last modified time of the item.
         *
         * Always do a full download.
         */
        bool m_ignoreLastModified;

        /**
         * The stream to use for writing to the standard output.
         *
         * This is used in verbose mode.
         */
        QTextStream *m_outStream;
        /**
         * The stream to use for writing error messages.
         */
        QTextStream *m_errStream;

        /**
         * Check if an episode has been downloaded and set episodes as
         * downloaded.
         */
        Database *m_database;

        /**
         * Starts downloads of DownloadItems.
         */
        QNetworkAccessManager *m_networkAccessManager;
        /**
         * The number of currently downloading objects. When this reaches 0
         * the application will exit.
         */
        int m_activeDownloadCount;

        /**
         * Holds the settings used by the application.
         */
        SettingsManager *m_settingsManager;

        /**
         * Podcasts waiting to have their rss feeds downloaded.
         */
        QQueue<Podcast *> m_podcastRSSQueue;
        /**
         * Podcasts that have episodes that are waiting to be downloaded.
         */
        QQueue<Podcast *> m_podcastDownloadQueue;
};

#endif /* CLIENT_H */
