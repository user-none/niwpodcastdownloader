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
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>

#include <stdlib.h>

#include "client.h"
#include "configure.h"
#include "opts.h"
#include "platform.h"
#include "podcastlistingsparser.h"

Client::Client()
{
    m_errStream = new QTextStream(stderr);
    m_outStream = new QTextStream(stdout);
    m_database = new Database();
    m_networkAccessManager = new QNetworkAccessManager();
    m_activeDownloadCount = 0;
    m_settingsManager = new SettingsManager();
    m_initMode = false;
    m_verboseMode = false;
    m_ignoreLastModified = false;
}

Client::~Client()
{
    delete m_errStream;
    delete m_outStream;
    delete m_database;
    delete m_networkAccessManager;
    delete m_settingsManager;
}

void Client::run()
{
    // Any of these functions can cause the application to exit.
    parseOptions();
    loadDatabase();
    loadPodcasts();

    // Start downloading the rss feeds. The number of downloads started is
    // the minimum of the user defined download thread count and the number
    // of podcasts in the queue. Calling more threads than there are items
    // wouldn't do anything productive.
    for (int i = 0; i < qMin(m_settingsManager->getThreadCount(),
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
    *m_errStream << tr("Error: could not download %1 because %2")
        .arg(item->getName()).arg(errorString) << endl;

    m_activeDownloadCount--;
    downloadNext();

    item->deleteLater();
}

void Client::error(const QString &error, bool fatal)
{
    if (m_errStream) {
        *m_errStream << tr("Error: %1").arg(error) << endl;
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

    QNetworkRequest request = getNetworkRequest();

    // New podcast download.
    if (!podcast) {
        m_activeDownloadCount++;
        podcast = m_podcastRSSQueue.dequeue();
        url = podcast->getUrl();
        QString lastModified = m_database->getLastModified(podcast);

        if (!lastModified.isEmpty() && !m_ignoreLastModified) {
            request.setRawHeader("If-Modified-Since", lastModified.toAscii());
        }

        connect(podcast, SIGNAL(contentMoved(DownloadItem *, QUrl)), this,
            SLOT(startRSSDownload(DownloadItem *, QUrl)));

        // downloadError, downloadItemNotModified, and finished are exclusive.
        // Only one will be called.
        connect(podcast, SIGNAL(error(DownloadItem *, QString)), this,
            SLOT(downloadError(DownloadItem *, QString)));
        connect(podcast, SIGNAL(notModified(DownloadItem *)), this,
            SLOT(downloadItemNotModified(DownloadItem *)));
        connect(podcast, SIGNAL(finished(DownloadItem *)), this,
            SLOT(episodesReady(DownloadItem *)));
    }

    verbose(tr("Starting rss download for %1 from %2.").arg(podcast->getName())
        .arg(url.toString()));

    QNetworkReply *reply;

    // Start the download.
    request.setUrl(url);
    reply = m_networkAccessManager->get(request);
    podcast->setNetworkReply(reply);
}

void Client::episodesReady(DownloadItem *item)
{
    // item is really a Podcast object. The signal is set in the base class
    // hence why there must be a cast to the derived class type.
    Podcast *podcast = static_cast<Podcast *>(item);

    verbose(tr("Rss download finished for %1.").arg(podcast->getName()));

    if (podcast->isInit() || m_initMode) {
        // Mark all episodes as downloaded.
        Q_FOREACH (PodcastEpisode *episode, podcast->getEpisodes()) {
            // Do not mark explicit episodes as downloaded when filtering
            // explicit. We don't want the user to think that explicit episodes
            // are being downloaded when they've requested not to.
            if (!m_settingsManager->getFilterExplicit()
                || !episode->isExplicit())
            {
                m_database->setDownloaded(episode);
            }
        }
        podcast->clearEpisodeList();
        podcast->deleteLater();

        verbose(tr("Running in init mode. Marking all episodes for %1 as"
            " downloaded.").arg(podcast->getName()));
    }
    else {
        // Generate a list of episodes to download.
        podcast->truncateEpisodes(m_settingsManager->getRecentEpisodeCount());
        Q_FOREACH (PodcastEpisode *episode, podcast->getEpisodes()) {
            // Remove downloaded and explicit if we are filtering explicit.
            if (m_database->isDownloaded(episode)
                || (m_settingsManager->getFilterExplicit()
                && episode->isExplicit()))
            {
                podcast->removeEpisode(episode);
            }
        }

        verbose(tr("Queuing %1 episodes from %2 for download.")
            .arg(podcast->getEpisodeCount()).arg(podcast->getName()));

        if (podcast->getEpisodeCount() > 0) {
            m_podcastDownloadQueue.enqueue(podcast);
        }
        else {
            // Set the modified date for the rss feed. We are setting it here
            // becuase there are no episodes to download. Otherwise the date
            // will be set when the last episode download for the particular
            // podcast starts.
            m_database->setLastModified(podcast);

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

    QNetworkRequest request = getNetworkRequest();

    // New episode download.
    if (!episode) {
        Podcast *podcast = m_podcastDownloadQueue.dequeue();

        episode = podcast->takeFirstEpisode();
        url = episode->getUrl();

        QDir fileDirectory(QString("%1/%2/%3")
            .arg(m_settingsManager->getSaveLocation())
            .arg(podcast->getCategory())
            .arg(podcast->getName()));

        // create the directory to download to.
        if (!fileDirectory.exists()) {
            if (!fileDirectory.mkpath(fileDirectory.path())) {
                error(tr("Could not create directory: %1 to write %2 for %3.")
                    .arg(fileDirectory.path())
                    .arg(QFileInfo(episode->getUrl().toString()).fileName())
                    .arg(episode->getName()),
                    false);

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
        episode->setSaveLocation(QString("%1/%2")
            .arg(fileDirectory.absolutePath())
            .arg(QFileInfo(episode->getUrl().toString()).fileName()));

        if (podcast->getEpisodeCount() > 0) {
            m_podcastDownloadQueue.enqueue(podcast);
        }
        else {
            // Set the modified date for the rss feed.
            // TODO: What happens if the download fail? It won't be
            // re-downloaded until next time there are new items.
            m_database->setLastModified(podcast);

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
        // TODO:
        // re-calculate the file name and change the download file name.
    }

    verbose(tr("Starting episode download for %1 from %2 and saving to %3.")
        .arg(episode->getName()).arg(url.toString())
        .arg(episode->getSaveLocation()));

    QNetworkReply *reply;

    // Start the download.
    request.setUrl(url);
    reply = m_networkAccessManager->get(request);
    episode->setNetworkReply(reply);
}

void Client::episodeDownloaded(DownloadItem *item)
{
    // item is really a PodcastEpisode object. The signal is set in the base
    // class hence why there must be a cast to the derived class type.
    PodcastEpisode *episode = static_cast<PodcastEpisode *>(item);

    verbose(tr("Episode %1 downloaded successfully.").arg(episode->getName()));

    m_database->setDownloaded(episode);

    episode->deleteLater();

    m_activeDownloadCount--;
    downloadNext();
}

void Client::downloadItemNotModified(DownloadItem *item)
{
    verbose(tr("%1 at %2 has not been modified since the last time it was"
        "downloaded.").arg(item->getName()).arg(item->getUrl().toString()));

    item->deleteLater();

    m_activeDownloadCount--;
    downloadNext();
}


void Client::loadDatabase()
{
    connect(m_database, SIGNAL(error(const QString &, bool)), this,
        SLOT(error(const QString &, bool)));

    verbose(tr("Opening database at %1.").arg(m_settingsManager
        ->getDatabaseFile()));
    // If the file cannot be opened it is fatal.
    if (!m_database->open(m_settingsManager->getDatabaseFile())) {
        error(m_database->openError(), true);
    }
}

void Client::loadPodcasts()
{
    verbose(tr("Opening podcast listings file at %1.").arg(m_settingsManager
        ->getPodcastsFile()));
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

    verbose(tr("Found %1 podcasts.").arg(m_podcastRSSQueue.size()));

    // Exit if there are no podcasts.
    if (m_podcastRSSQueue.size() == 0) {
        exit(0);
    }
}

QNetworkRequest Client::getNetworkRequest()
{
    QNetworkRequest request;

    request.setRawHeader("User-Agent", QString("%1 %2")
        .arg(QCoreApplication::applicationName())
        .arg(Configure::applicationVersion).toAscii());

    return request;
}

void Client::verbose(const QString &message)
{
    if (m_verboseMode) {
        *m_outStream << "Verbose: " << message << endl;
    }
}

void Client::parseOptions()
{
    OptsOption initOption(tr("init"), &m_initMode, false, 0,
        tr("Init mode. Mark all episodes as downloaded without downloading "
        "any."), "");

    OptsOption verboseOption(tr("verbose"), &m_verboseMode, false, 0,
        tr("Verbose mode. Be chatty about what is happening."), "");

    bool writeConfigSet = false;
    OptsOption writeConfigOption(tr("write_config"), &writeConfigSet, false, 0,
        tr("Write the default config to disk. This will over write any "
        "existing configuration settings."), "");

    bool filterExplicit = false;
    OptsOption filterExplicitOption(tr("filter_explicit"), &filterExplicit,
        false, 0, tr("Do not download episodes marked as explicit"), "");

    OptsOption ignoreLastModifiedOption(tr("ignore_not_modified"),
        &m_ignoreLastModified, false, 0, tr("Do a full download of all rss"
        " feeds. Do not rely on the last modified time the server reports to"
        " determine if there are no new episodes."), "");

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
    opts.addOption(ignoreLastModifiedOption);
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
    if (filterExplicit) {
        m_settingsManager->setFilterExplicit(true);
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
