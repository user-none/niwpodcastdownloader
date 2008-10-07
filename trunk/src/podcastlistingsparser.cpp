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
#include <QFile>

#include "podcastlistingsparser.h"

void PodcastListingsParser::parseListingsFile(const QString &listingsFile)
{
    m_podcasts.clear();

    QFile file(listingsFile);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error(tr("Could not open file %1 because %2.").arg(listingsFile)
            .arg(file.errorString()), true);
        return;
    }

    QDomDocument document;
    QString errorMsg;
    int errorLine;
    int errorColumn;

    if (!document.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
        emit error(
            tr("Could not parse %1 because %2 at line %3 and column %4.")
            .arg(listingsFile).arg(errorMsg).arg(errorLine).arg(errorColumn),
            true);
        file.close();
        return;
    }
    file.close();

    QDomElement rootElement = document.firstChildElement("podcasts");
    if (rootElement.isNull()) {
        emit error(tr("Root element <podcasts> not found in listings file %1.")
            .arg(listingsFile), false);
        return;
    }

    QDomElement itemElement = rootElement.firstChildElement("item");

    while (!itemElement.isNull()) {
        QDomElement dataElement = itemElement.firstChildElement();

        Podcast *podcast = new Podcast();

        while (!dataElement.isNull()) {
            if (dataElement.tagName().trimmed().toLower() == "name") {
                podcast->setName(dataElement.text());
            }
            else if (dataElement.tagName().trimmed().toLower() == "init") {
                podcast->setInit(true);
            }
            else if (dataElement.tagName().trimmed().toLower() == "category") {
                podcast->setCategory(dataElement.text());
            }
            else if (dataElement.tagName().trimmed().toLower() == "url") {
                podcast->setUrl(QUrl(dataElement.text()));
            }

            dataElement = dataElement.nextSiblingElement();
        }

        if (podcast->getName().isEmpty() || podcast->getUrl().isEmpty()
            || !podcast->getUrl().isValid())
        {
            delete podcast;
        }
        else {
            m_podcasts.append(podcast);
        }

        itemElement = itemElement.nextSiblingElement("item");
    }
}

QList<Podcast *> PodcastListingsParser::getPodcasts() const
{
    return m_podcasts;
}
