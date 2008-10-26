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

#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "podcast.h"
#include "podcastepisode.h"

/**
 * All database manipulation is handled by this class.
 *
 * The database holds non-settings data that needs to be accessed across
 * sessions. Database manipulation is encapsulated here to make it easier to
 * change the database type if necessary.
 */
class Database : public QObject
{
    Q_OBJECT

    public:
        Database();
        ~Database();

        /**
         * Open the database.
         *
         * @param file The db file to open.
         *
         * @return Ture on success.
         */
        bool open(const QString &file);
        /**
         * The error associated with a failed open.
         *
         * The error can include failure to open, update, or read. All other
         * errors, mainly sql query errors, will be emitted as an error signal.
         *
         * @return A human readable string representing the error if one has
         * occurred. Otherwise an empty string is returned.
         */
        QString openError();

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

        /**
         * Gets the last modified date of the rss feed.
         *
         * @param podcast The podcast to get the modified date for.
         *
         * @return The last modified date.
         */
        QString getLastModified(Podcast *podcast);
        /**
         * Sets the last modified date of the rss feed.
         *
         * @param podcast The podcast to set the modified date for.
         */
        void setLastModified(Podcast *podcast);

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
         * Create the default tables for an empty database.
         *
         * @return True if the database was created successfully.
         */
        bool createDefaultDb();
        /**
         * Verify the db is valid.
         *
         * @return True if the database is valid.
         */
        bool verifyDb();
        /**
         * Update the database to the latest version.
         *
         * @param version The current version of the db.
         *
         * @return True if the database was successfully updated.
         */
        bool updateDb(int version);
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

        /**
         * The error message associated with an error opening the database.
         */
        QString m_openError;

        /**
         * A magical id written to the db used to verify that the db is valid.
         */
        static const QString dbID;
        /**
         * The version of the db.
         *
         * Used to verify that this verison of the application can use the db.
         */
        static const int dbVersion;
};

#endif /* DATABASE_H */
