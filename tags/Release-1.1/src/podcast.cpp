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

#include <QDomDocument>
#include <QDomElement>
#include <QListIterator>

#include "podcast.h"

Podcast::Podcast()
{
    m_category = "";
    m_init = false;
    m_ignoreNotModified = false;
}

bool Podcast::isInit()
{
    return m_init;
}

bool Podcast::isIgnoreNotModified()
{
	return m_ignoreNotModified;
}

QString Podcast::getCategory() const
{
    return m_category;
}

QList<PodcastEpisode *> Podcast::getEpisodes() const
{
    return m_episodes;
}

int Podcast::getEpisodeCount()
{
    return m_episodes.size();
}

PodcastEpisode* Podcast::takeFirstEpisode()
{
    if (m_episodes.size() > 0) {
        return m_episodes.takeFirst();
    }
    return 0;
}

void Podcast::setInit(bool init)
{
    m_init = init;
}

void Podcast::setIgnoreNotModified(bool ignore)
{
	m_ignoreNotModified = ignore;
}

void Podcast::setCategory(const QString &category)
{
    m_category = category;
}

void Podcast::removeEpisode(PodcastEpisode *episode)
{
    m_episodes.removeAll(episode);
    delete episode;
    episode = 0;
}

void Podcast::truncateEpisodes(int position)
{
    // The list should not be negative nor should this remove all episodes.
    // Use clearEpisodeList to remove all episodes.
    if (position <= 0) {
        return;
    }

    while (m_episodes.size() > position) {
        PodcastEpisode *episode = m_episodes.takeLast();
        delete episode;
        episode = 0;
    }
}

void Podcast::clearEpisodeList()
{
    Q_FOREACH (PodcastEpisode *episode, m_episodes) {
        delete episode;
        episode = 0;
    }
    m_episodes.clear();
}

bool Podcast::downloadSuccessful()
{
    clearEpisodeList();

    if (!m_reply) {
        return false;
    }

    QDomDocument document;
    QString errorMsg;
    int errorLine;
    int errorColumn;

    if (!document.setContent(m_reply, &errorMsg, &errorLine, &errorColumn)) {
        emit error(this, tr("Could not parse %1 because %2 at line %3 and"
            " column %4.").arg(getName()).arg(errorMsg).arg(errorLine)
            .arg(errorColumn));
        return false;
    }

    QDomElement rootElement = document.firstChildElement("rss");
    if (rootElement.isNull()) {
        emit error(this, tr("Rss element <rss> not found in podcast feed for"
            " %1.").arg(getName()));
        return false;
    }
    QDomElement channelElement = rootElement.firstChildElement("channel");
    if (channelElement.isNull()) {
        emit error(this, tr("Rss element <channel> not found in podcast feed"
            " for %1.").arg(getName()));
        return false;
    }

    QDomElement itemElement = channelElement.firstChildElement("item");

    while (!itemElement.isNull()) {
        QDomElement dataElement = itemElement.firstChildElement();

        PodcastEpisode *episode = new PodcastEpisode();

        while (!dataElement.isNull()) {
            if (dataElement.tagName().trimmed().toLower() == "title") {
                episode->setName(dataElement.text());
            }
            else if (dataElement.tagName().trimmed().toLower() == "pubdate") {
                QString pubdate = dataElement.text().trimmed();
                pubdate.truncate(pubdate.lastIndexOf(" "));
                episode->setPublishDate(QDateTime::fromString(
                    pubdate, "ddd, dd MMM yyyy HH:mm:ss"));
            }
            else if (dataElement.tagName().trimmed().toLower() == "enclosure")
            {
                episode->setUrl(QUrl(dataElement.attribute("url")));
            }
            else if (dataElement.tagName().trimmed().toLower()
                == "itunes:explicit")
            {
                // We want to be conservative. If it's not no it's assumed to
                // be yes. We don't need to set the explicit status to false
                // because false is the default status for an episode.
                if (dataElement.attribute("url") != "no") {
                    episode->setExplicit(true);
                }
            }

            dataElement = dataElement.nextSiblingElement();
        }

        if (episode->getUrl().isEmpty() || !episode->getUrl().isValid()) {
            delete episode;
            episode = 0;
        }
        else {
            m_episodes.append(episode);
        }

        itemElement = itemElement.nextSiblingElement("item");
    }

    // Sort the episodes so that the most recent are first. This is done so
    // that truncateEpisodes removes older episodes rather than newer ones.
    // m_episodes is a list of pointers to the greaterThan function must
    // be given otherwise qSort will sort based upon the memory address of
    // the pointers not the episode objects.
    qSort(m_episodes.begin(), m_episodes.end(), PodcastEpisode::greaterThan);

    return true;
}
