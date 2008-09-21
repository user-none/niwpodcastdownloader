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

#ifndef DOWNLOADITEM_H
#define DOWNLOADITEM_H

#include <QNetworkReply>
#include <QObject>
#include <QStringList>
#include <QUrl>

/**
 * An item that can be downloaded.
 *
 * The base class for Podcast and PodcastEpisode. Implements their shared
 * functionality for downloading.
 */
class DownloadItem : public QObject
{
    Q_OBJECT

    public:
        DownloadItem();

        /**
         * Gets the name of the item.
         *
         * @return The name of the item.
         */
        QString getName() const;
        /**
         * Gets the url associated with the item.
         *
         * The url is what the download item is able to download.
         *
         * @return The url of the item.
         */
        QUrl getUrl() const;

        /**
         * Sets the name of the item.
         *
         * @param name The name of the item.
         */
        void setName(const QString &name);
        /**
         * Sets the URL of the item.
         *
         * @param url The url of the rss feed.
         */
        void setUrl(const QUrl &url);

        /**
         * Sets the network reply used for downloading the item.
         *
         * The reply is used for singling the state of the download.
         *
         * @param *reply The network reply returned by a QNetworkAccessManager
         * object.
         *
         * @see downloadFinished
         */
        void setNetworkReply(QNetworkReply *reply);

    public slots:
        /**
         * The requested content has finished downloading.
         *
         * This will processes the response codes and either emit
         * an error or contentMoved signal. Or will call downloadSuccessful.
         *
         * @see error
         * @see contentMoved
         * @see downloadSuccessful
         */
        void downloadFinished();

    signals:
        /**
         * This signal is emitted if the requested download has moved to a new
         * location.
         *
         * @param item this.
         * @param url The new url that the content is located at.
         */
        void contentMoved(DownloadItem *item, QUrl url);
        /**
         * This signal is emitted when there is an error condition.
         *
         * @param item this.
         * @param error The error associated with the failed download.
         */
        void error(DownloadItem *item, QString error);
        /**
         * This signal is emitted if the requested download has finished.
         *
         * @param item this.
         */
        void finished(DownloadItem *item);

    protected:
        /**
         * Derived classes can do any needed processing on the downloaded
         * content.
         *
         * @return True if the processing completes successfully. False if
         * there was an error.
         */
        virtual bool downloadSuccessful() = 0;
        /**
         * Clean up any resources used by the download.
         *
         * This is necessary if the download is stopped before it completes.
         */
        virtual void cleanDownload();

        /**
         * The network reply associated with the download.
         */
        QNetworkReply *m_reply;

    private:
        /**
         * The name of the item.
         */
        QString m_name;
        /**
         * The url to download.
         */
        QUrl m_url;

        /**
         * A list of urls used with 301 and 302 content moved responses.
         *
         * This list is used to see if the redirect will cause an infinate
         * loop.
         */
        QStringList m_rssMovedUrls;
};

#endif /* DOWNLOADITEM_H */
