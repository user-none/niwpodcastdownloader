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

#ifndef EPISODELISTING_H
#define EPISODELISTING_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "podcastepisode.h"

/**
 * Get and set the status of a PodcastEpisode.
 *
 * Checks if a PodcastEpisode has been download. Also, can set if a
 * PodcastEpisode as having been downloaded. Checking and setting is based
 * on the url associated with the episode.
 *
 * This class is not thread safe. This is because Qt does not allow SQL
 * connections to be moved between threads.
 */
class EpisodeListing : public QObject
{
    Q_OBJECT

    public:
        EpisodeListing();
        ~EpisodeListing();

        /**
         * Open the downloaded episode db.
         *
         * @param file The db file to open.
         *
         * @return Ture on success.
         */
        bool open(const QString &file);
        /**
         * Check if a PodcastEpisode has been previously downloaded.
         *
         * @param episode The podcast episode to check.
         *
         * @return True if the episode has previously been downloaded. 
         */
        bool isDownloaded(PodcastEpisode *episode);
        /**
         * Sets the episode as having been downloaded in the episode db.
         *
         * @param episode The episode to set as downloaded.
         */
        void setDownloaded(PodcastEpisode *episode);

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
         * Executes a SQLite query.
         *
         * @param query The query to execute.
         *
         * @return True if the query was successfully executed.
         */
        bool execQuery(const QString &query);

        /**
         * The database connection.
         */
        QSqlDatabase m_db;
        /**
         * The name of the database.
         */
        QString m_dbName;
        /**
         * Object used for executing queries on the database.
         */
        QSqlQuery *m_query;
};

#endif /* EPISODELISTING_H */
