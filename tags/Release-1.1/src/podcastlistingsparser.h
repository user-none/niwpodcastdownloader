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

#ifndef PODCASTLISTINGSPARSER_H
#define PODCASTLISTINGSPARSER_H

#include <QObject>

#include "podcast.h"

/**
 * Parses an xml file containing a list of podcasts to download.
 *
 * Note when destroyed, the podcast object will NOT delete any associated
 * PodcastEpisode objects.
 */
class PodcastListingsParser : public QObject
{
    Q_OBJECT

    public:
        /**
         * Parses an xml file generating a list of podcasts.
         *
         * This will not delete any current podcast items in the list.
         * They must be deleted manually before subsequent calls to this
         * function. Otherwise, this will cause a memory leak.
         *
         * @param listingsFile The file name and path to the file to parse.
         */
        void parseListingsFile(const QString &listingsFile);
        /**
         * The list of podcasts.
         *
         * The list is populated by calling parseListingsFile.
         *
         * @see parseListingsFile
         */
        QList<Podcast *> getPodcasts() const;

    signals:
        /**
         * This signal is emitted when there is an error condition.
         *
         * @param error The error message.
         * @param fatal True if this is a fatal error and the application
         * should exit.
         */
        void error(const QString &error, bool fatal);

    private:
        /**
         * The list of podcasts.
         */
        QList<Podcast *> m_podcasts;
};

#endif /* PODCASTLISTINGSPARSER_H */
