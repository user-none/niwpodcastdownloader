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

#include "downloaditem.h"

DownloadItem::DownloadItem()
{
    m_name = "";
    m_url.clear();
    m_reply = 0;
}

QString DownloadItem::getName() const
{
    return m_name;
}

QUrl DownloadItem::getUrl() const
{
    return m_url;
}

void DownloadItem::setName(const QString &name)
{
    m_name = name;
}

void DownloadItem::setUrl(const QUrl &url)
{
    m_url = url;
}

void DownloadItem::setNetworkReply(QNetworkReply *reply)
{
    // Disconnect any signals if a reply was previously set. A new reply may
    // be set over an old one in content moved instances.
    if (m_reply) {
        disconnect(m_reply, SIGNAL(finished()), this,
            SLOT(downloadFinished()));
        m_reply->deleteLater();
    }

    m_reply = reply;
    // If the reply is not deleted elsewhere we want the reply to be deleted
    // when this is deleted.
    m_reply->setParent(this);
    // The name should not be seen by the end user. The name of the reply will
    // be written to stderr if there is an issue with the object. This could
    //  happen if the reply is deleted while in it's event handler.
    m_reply->setObjectName("reply for: " + getName());

    connect(m_reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(m_reply, SIGNAL(sslErrors(const QList<QSslError> &)), m_reply,
        SLOT(ignoreSslErrors()));
}

void DownloadItem::downloadFinished()
{
    if (!m_reply) {
        return;
    }

    if (m_reply->error() != QNetworkReply::NoError) {
        cleanDownload();
        emit error(this, "Connection failed because "
            + m_reply->errorString());
        return;
    }
    if (m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).isNull())
    {
        cleanDownload();
        emit error(this, "Connection failed because no HTTP status code was"
            " returned");
        return;
    }

    switch (m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
        .toInt())
    {
        // OK
        // Everything is fine, the download completed successfully.
        case 200:
            // downloadSuccessful must be called before cleanDownload.
            // cleanDownload must be called before the finished signal is
            // emitted.
            if (downloadSuccessful()) {
                cleanDownload();
                emit finished(this);
            }
            else {
                cleanDownload();
            }
            break;
        // Moved Permanently
        case 301:
            // Fall though wanted.
        // Moved Temporarily
        case 302: {
            QUrl newUrl = m_reply->attribute(
                QNetworkRequest::RedirectionTargetAttribute).toUrl();

            if (!m_rssMovedUrls.contains(newUrl.toString())) {
                m_rssMovedUrls.append(newUrl.toString());
                DownloadItem::cleanDownload();
                emit contentMoved(this, newUrl);
            }
            else {
                cleanDownload();
                emit error(this, tr("Infinite redirection"));
            }
            break;
        }
        // Not Modified
        case 304: {
        }
        // Other HTTP Status codes.
        default:
            cleanDownload();
            emit error(this, tr("Http status") + " "
                + m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                .toString() + ": " + m_reply->attribute(
                QNetworkRequest::HttpReasonPhraseAttribute).toString());
            break;
    }
}

void DownloadItem::cleanDownload()
{
    if (m_reply) {
        disconnect(m_reply, SIGNAL(finished()), this,
            SLOT(downloadFinished()));

        m_reply->close();
        m_reply->deleteLater();
        m_reply = 0;
    }
}
