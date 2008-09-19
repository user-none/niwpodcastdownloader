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

#include <QCoreApplication>
#include <QDir>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>

#include <stdlib.h>

#include "client.h"
#include "opts.h"
#include "platform.h"
#include "podcastlistingsparser.h"

Client::Client()
{
    m_errStream = new QTextStream(stderr);
    m_outStream = new QTextStream(stdout);
    m_episodeListing = new EpisodeListing();
    m_networkAccessManager = new QNetworkAccessManager();
    m_activeDownloadCount = 0;
    m_settingsManager = new SettingsManager();
    m_initMode = false;
    m_verboseMode = false;
}

Client::~Client()
{
    delete m_errStream;
    delete m_outStream;
    delete m_episodeListing;
    delete m_networkAccessManager;
    delete m_settingsManager;
}

void Client::run()
{
    parseOptions();

    connect(m_episodeListing, SIGNAL(error(const QString &, bool)), this,
        SLOT(error(const QString &, bool)));

    verbose(tr("Opening episodes database at") + " "
        + m_settingsManager->getDownloadedEpisodeListFile() + tr("."));
    // If the file cannot be opened a fatal error will be emitted and the
    // application will exit.
    m_episodeListing->open(m_settingsManager->getDownloadedEpisodeListFile());

    verbose(tr("Opening podcast listings file at") + " "
        + m_settingsManager->getPodcastsFile() + tr("."));
    // Get the podcast rss urls from the listing file.
    PodcastListingsParser podcastListingsParser;
    connect(&podcastListingsParser, SIGNAL(error(const QString &, bool)),
        this, SLOT(error(const QString &, bool)));
    podcastListingsParser.parseListingsFile(
        m_settingsManager->getPodcastsFile());

    // Add the valid podcasts to the rss queue so their rss feeds can be
    // downloaded.
    Q_FOREACH (Podcast *podcast, podcastListingsParser.getPodcasts()) {
        m_podcastRSSQueue.enqueue(podcast);
    }

    verbose(tr("Found") + " " + QString::number(m_podcastRSSQueue.size()) + " "
        + tr("podcasts."));

    // Exit if there are no podcasts.
    if (m_podcastRSSQueue.size() == 0) {
        exit(0);
    }

    // Start downloading the rss feeds. The number of downloads started is
    // the minimum of the user defined download thread count and the number
    // of podcasts in the queue. Calling more threads than there are items
    // wouldn't do anything productive.
    for (int i = 0; i < minimum(m_settingsManager->getThreadCount(),
        m_podcastRSSQueue.size()); i++)
    {
        startRSSDownload();
    }
}

void Client::downloadNext()
{
    // Check disk space requirements
    if (m_settingsManager->getMinimumFreeDiskSpace() < 0
        ||
        Platform::getFreeDiskSpace(m_settingsManager->getSaveLocation()) <= -1
        || Platform::getFreeDiskSpace(m_settingsManager->getSaveLocation())
        > m_settingsManager->getMinimumFreeDiskSpace())
    {
        if (!m_podcastRSSQueue.isEmpty()) {
            startRSSDownload();
        }
        else if (!m_podcastDownloadQueue.isEmpty()) {
            startEpisodeDownload();

            // This takes care of cases where there are less podcasts than
            // threads. So the number of threads started is the number of
            // podcasts. Podcasts can have more episodes than the number of
            // podcasts. This will start more downloads to bring it up to the
            // number set by the user.
            while (m_activeDownloadCount < m_settingsManager->getThreadCount()
                && !m_podcastDownloadQueue.isEmpty())
            {
                startEpisodeDownload();
            }
        }
    }
    // Not enough free disk space to continue downloading.
    else {
        error(tr("Not enough free space to continue downloading."), false);
        // Empty the queues. Once any current downloads finish the application
        // will exit.
        while (!m_podcastRSSQueue.isEmpty()) {
            m_podcastRSSQueue.dequeue()->deleteLater();
        }
        while (!m_podcastDownloadQueue.isEmpty()) {
            Podcast *podcast = m_podcastRSSQueue.dequeue();
            podcast->clearEpisodeList();
            podcast->deleteLater();
        }
    }

    // If there are no active downloads, exit.
    if (m_activeDownloadCount == 0) {
        exit(0);
    }
}

void Client::downloadError(DownloadItem *item, QString errorString)
{
    *m_errStream << tr("Error: could not download") << " " << item->getName()
        << ", " << tr("because") << " " << errorString << endl;

    m_activeDownloadCount--;
    downloadNext();

    item->deleteLater();
}

void Client::error(const QString &error, bool fatal)
{
    if (m_errStream) {
        *m_errStream << tr("Error:") << " " << error << endl;
    }

    if (fatal) {
        *m_errStream << tr("Fatal: Exiting.") << endl;
        exit(1);
    }
}

void Client::startRSSDownload(DownloadItem *item, QUrl url)
{
    // item is really a Podcast object. The signal is set in the base class
    // hence why there must be a cast to the derived class type.
    Podcast *podcast = static_cast<Podcast *>(item);

    // New podcast download.
    if (!podcast) {
        m_activeDownloadCount++;
        podcast = m_podcastRSSQueue.dequeue();
        url = podcast->getUrl();

        connect(podcast, SIGNAL(contentMoved(DownloadItem *, QUrl)), this,
            SLOT(startRSSDownload(DownloadItem *, QUrl)));
        connect(podcast, SIGNAL(error(DownloadItem *, QString)), this,
            SLOT(downloadError(DownloadItem *, QString)));
        connect(podcast, SIGNAL(finished(DownloadItem *)), this,
            SLOT(episodesReady(DownloadItem *)));
    }

    verbose(tr("Starting rss download for") + " " + podcast->getName() + " "
        + tr("from") + " " + url.toString() + tr("."));

    QNetworkReply *reply;

    // Start the download.
    reply = m_networkAccessManager->get(QNetworkRequest(url));
    podcast->setNetworkReply(reply);
}

void Client::episodesReady(DownloadItem *item)
{
    // item is really a Podcast object. The signal is set in the base class
    // hence why there must be a cast to the derived class type.
    Podcast *podcast = static_cast<Podcast *>(item);

    verbose(tr("Rss download finished for") + " " + podcast->getName()
        + tr("."));

    if (podcast->isInit() || m_initMode) {
        // Mark all episodes as downloaded.
        Q_FOREACH (PodcastEpisode *episode, podcast->getEpisodes()) {
            m_episodeListing->setDownloaded(episode);
        }
        podcast->clearEpisodeList();
        podcast->deleteLater();

        verbose(tr("Running in init mode. Marking all episodes for") + " "
            + podcast->getName() + " " + tr("as downloaded."));
    }
    else {
        // Generate a list of episodes to download.
        podcast->truncateEpisodes(m_settingsManager->getRecentEpisodeCount());
        Q_FOREACH (PodcastEpisode *episode, podcast->getEpisodes()) {
            if (m_episodeListing->isDownloaded(episode)) {
                podcast->removeEpisode(episode);
            }
        }

        verbose(tr("Queuing") + " "
            + QString::number(podcast->getEpisodeCount()) + " "
            + tr("episodes from") + " " + podcast->getName() + " "
            + tr("for download."));

        if (podcast->getEpisodeCount() > 0) {
            m_podcastDownloadQueue.enqueue(podcast);
        }
        else {
            podcast->deleteLater();
        }
    }

    m_activeDownloadCount--;
    downloadNext();
}

void Client::startEpisodeDownload(DownloadItem *item, QUrl url)
{
    // item is really a PodcastEpisode object. The signal is set in the base
    // class hence why there must be a cast to the derived class type.
    PodcastEpisode *episode = static_cast<PodcastEpisode *>(item);

    // New episode download.
    if (!episode) {
        Podcast *podcast = m_podcastDownloadQueue.dequeue();

        episode = podcast->takeFirstEpisode();
        url = episode->getUrl();

        QDir fileDirectory(m_settingsManager->getSaveLocation() + "/"
            + podcast->getCategory() + "/" + podcast->getName());

        // create the directory to download to.
        if (!fileDirectory.exists()) {
            if (!fileDirectory.mkpath(fileDirectory.path())) {
                error(tr("Could not create directory:") + " "
                    + fileDirectory.path() + ", " + tr("to write") + " "
                    + episode->getUrl().path(), false);

                podcast->deleteLater();

                if (podcast->getEpisodeCount() > 0) {
                    podcast->clearEpisodeList();
                    podcast->deleteLater();
                }

                downloadNext();
                return;
            }
        }

        // Tell the episode where to download to.
        episode->setSaveLocation(fileDirectory.absolutePath() + "/"
            + episode->getUrl().path()
            .remove(0, episode->getUrl().path().lastIndexOf("/") + 1));

        if (podcast->getEpisodeCount() > 0) {
            m_podcastDownloadQueue.enqueue(podcast);
        }
        else {
            podcast->deleteLater();
        }

        connect(episode, SIGNAL(contentMoved(DownloadItem *, QUrl)), this,
            SLOT(startEpisodeDownload(DownloadItem *, QUrl)));
        connect(episode, SIGNAL(error(DownloadItem *, QString)), this,
            SLOT(downloadError(DownloadItem *, QString)));
        connect(episode, SIGNAL(finished(DownloadItem *)), this,
            SLOT(episodeDownloaded(DownloadItem *)));

        m_activeDownloadCount++;
    }
    else {
        // Set the write buffer for the episode to the beginning of the file.
        episode->resetWrite();
    }

    verbose(tr("Starting episode download for") + " " + episode->getName()
        + " " + tr("from") + " " + url.toString() + " "
        + tr(" and saving to") + " " + episode->getSaveLocation() + tr("."));

    QNetworkReply *reply;

    // Start the download.
    reply = m_networkAccessManager->get(QNetworkRequest(url));
    episode->setNetworkReply(reply);
}

void Client::episodeDownloaded(DownloadItem *item)
{
    // item is really a PodcastEpisode object. The signal is set in the base
    // class hence why there must be a cast to the derived class type.
    PodcastEpisode *episode = static_cast<PodcastEpisode *>(item);

    verbose(tr("Episode") + " " + episode->getName() + " "
        + tr("downloaded successfully."));

    m_episodeListing->setDownloaded(episode);

    episode->deleteLater();

    m_activeDownloadCount--;
    downloadNext();
}

void Client::verbose(const QString &message)
{
    if (m_verboseMode) {
        *m_outStream << message << endl;
    }
}

void Client::parseOptions()
{
    OptsOption initOption(tr("init"), &m_initMode, false, 0,
        tr("Init mode. Mark all episodes as downloading without downloading "
        "any."), "");

    OptsOption verboseOption(tr("verbose"), &m_verboseMode, false, 0,
        tr("Verbose mode. Be chatty about what is happening."), "");

    bool writeConfigSet = false;
    OptsOption writeConfigOption(tr("write_config"), &writeConfigSet, false, 0,
        tr("Write the default config to disk. This will over write any "
        "existing configuration settings."), "");

    bool episodesdbSet = false;
    QString episodesdbArg = "";
    OptsOption episodesdbOption(tr("episodes_db"), &episodesdbSet, true,
        &episodesdbArg, tr("The episodes database to use."), tr("FILE"));

    bool saveLocationSet = false;
    QString saveLocationArg = "";
    OptsOption saveLocationOption(tr("save_location"), &saveLocationSet, true,
        &saveLocationArg, tr("The locaction to save downloaded episodes."),
        tr("PATH"));

    bool threadsSet = false;
    QString threadsArg = "";
    OptsOption threadsOption(tr("threads"), &threadsSet, true, &threadsArg,
        tr("Number of simultaneous downloads."), tr("NUMBER"));

    bool recentSet = false;
    QString recentArg = "";
    OptsOption recentOption(tr("recent"), &recentSet, true, &recentArg,
        tr("Maximum number of recent episodes to download. 0 for all recent "
        "episodes."), tr("NUMBER"));

    bool minFreeSet = false;
    QString minFreeArg = "";
    OptsOption minFreeOption(tr("min_free_space"), &minFreeSet, true,
        &minFreeArg, tr("Minimum amount of free disk space that must be free "
        "in order to start a download. This amount is in KB."), tr("NUMBER"));

    bool listingSet = false;
    QString listingArg = "";
    OptsOption listingOption(tr("listings_file"), &listingSet, true,
        &listingArg, tr("XML listing of podcasts to download."), tr("FILE"));

    Opts opts;

    opts.addOption(initOption);
    opts.addOption(verboseOption);
    opts.addOption(writeConfigOption);
    opts.addOption(episodesdbOption);
    opts.addOption(saveLocationOption);
    opts.addOption(threadsOption);
    opts.addOption(recentOption);
    opts.addOption(minFreeOption);
    opts.addOption(listingOption);

    QStringList arguments = QCoreApplication::arguments();
    arguments.removeAt(0);

    opts.parseOptions(arguments);

    if (writeConfigSet) {
        m_settingsManager->writeDefaultConfig();
    }
    if (episodesdbSet) {
        m_settingsManager->setDownloadedEpisodeListFile(episodesdbArg);
    }
    if (saveLocationSet) {
        m_settingsManager->setSaveLocation(saveLocationArg);
    }
    if (threadsSet) {
        m_settingsManager->setThreadCount(threadsArg.toInt());
    }
    if (recentSet) {
        m_settingsManager->setRecentEpisodeCount(recentArg.toInt());
    }
    if (minFreeSet) {
        m_settingsManager->setMinimumFreeDiskSpace(minFreeArg.toLongLong());
    }
    if (listingSet) {
        m_settingsManager->setPodcastsFile(listingArg);
    }
}

int Client::minimum(int a, int b)
{
    if (a < b) {
        return a;
    }
    return b;
}
