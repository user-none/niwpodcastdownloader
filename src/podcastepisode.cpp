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

#include "podcastepisode.h"

PodcastEpisode::PodcastEpisode()
{
    m_file = 0;
}

PodcastEpisode::~PodcastEpisode()
{
    if (m_file) {
        m_file->close();
        delete m_file;
    }
}

QDateTime PodcastEpisode::getPublishDate() const
{
    return m_publishDate;
}

void PodcastEpisode::setPublishDate(const QDateTime &date)
{
    m_publishDate = date;
}

void PodcastEpisode::setSaveLocation(const QString &fileName)
{
    m_fileName = fileName;

    if (m_file) {
        m_file->close();
        delete m_file;
    }

    m_file = new QFile(m_fileName);
    if (!m_file->open(QIODevice::WriteOnly)) {
        delete m_file;
        m_file = 0;
    }
}

void PodcastEpisode::resetWrite()
{
    if (m_file) {
        m_file->reset();
    }
}

void PodcastEpisode::setNetworkReply(QNetworkReply *reply)
{
    // This function is nearly identical to it's base class DownloadItem.
    // Except, for the addition of the readyRead signal from m_reply beining
    // handled.

    if (m_reply) {
        disconnect(m_reply, SIGNAL(readyRead()), this, SLOT(writeData()));
        disconnect(m_reply, SIGNAL(finished()), this,
            SLOT(downloadFinished()));
        m_reply->deleteLater();
    }

    m_reply = reply;
    m_reply->setParent(this);
    m_reply->setObjectName("reply for: " + getName());

    connect(m_reply, SIGNAL(readyRead()), this, SLOT(writeData()));
    connect(m_reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(m_reply, SIGNAL(sslErrors(const QList<QSslError> &)), m_reply,
        SLOT(ignoreSslErrors()));
}

void PodcastEpisode::writeData()
{
    if (m_file && m_file->isOpen()) {
        m_file->write(m_reply->readAll());
    }
    else {
        cleanDownload();
        emit error(this, tr("File") + ", " + m_fileName
            + " " + tr("could not opened for writing."));
    }
}

bool PodcastEpisode::greaterThan(PodcastEpisode *ep1, PodcastEpisode *ep2)
{
    if (ep1->getPublishDate() > ep2->getPublishDate()) {
        return true;
    }
    return false;
}

void PodcastEpisode::downloadSuccessful()
{
    if (m_file) {
        m_file->close();
        delete m_file;
        m_file = 0;
    }
    if (m_reply) {
        m_reply->close();
        m_reply = 0;
    }
}

void PodcastEpisode::cleanDownload()
{
    if (m_reply) {
        disconnect(m_reply, SIGNAL(readyRead()), this, SLOT(writeData()));
        disconnect(m_reply, SIGNAL(finished()), this,
            SLOT(downloadFinished()));

        m_reply->close();
        m_reply->deleteLater();
        m_reply = 0;
    }

    if (m_file) {
        m_file->remove();
        delete m_file;
        m_file = 0;
    }
}
