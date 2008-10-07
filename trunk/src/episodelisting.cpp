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

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QUuid>
#include <QVariant>

#include "episodelisting.h"

EpisodeListing::EpisodeListing()
{
    m_db = QSqlDatabase();
    m_dbName = "";
    m_query = 0;
}

EpisodeListing::~EpisodeListing()
{
    if (m_query) {
        delete m_query;
    }
    m_query = 0;

    m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_dbName);
}

bool EpisodeListing::open(const QString &file)
{
    // If the db doesn't exit we will need to create the default table later.
    bool newFile = false;

    QFileInfo fileInfo(file);

    // We can't open directories.
    if (fileInfo.isDir()) {
        emit error(tr("Cannot open file %1 because it is a directory.")
            .arg(file), true);
        return false;
    }
    if (!fileInfo.exists()) {
        newFile = true;

        // Create the containing directory if it doens't exist.
        // If the directory doesn't exist we can't create the db file in it.
        QDir fileDirectory = fileInfo.absoluteDir();
        if (!fileDirectory.exists()) {
            fileDirectory.mkpath(fileInfo.absolutePath());
        }
    }

    // Setup the database connection.
    m_dbName = QUuid::createUuid().toString();
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_dbName);
    m_db.setDatabaseName(file);

    if (!m_db.open()) {
        emit error(tr("Cannot open file %1 because %2.").arg(file)
            .arg(m_db.lastError().text()), true);
        return false;
    }

    // Associate a new query obecjt with the database.
    if (m_query) {
        delete m_query;
    }
    m_query = new QSqlQuery(m_db);

    // Create the default table if it's a new file.
    if (newFile) {
        if (!execQuery("CREATE TABLE episodes (url TEXT);")) {
            return false;
        }
    }

    // Check if the file is a valid episodes database.
    if (!execQuery("SELECT * FROM episodes")) {
        emit error(tr("Could not open file %1 because it is not a valid"
            " episode list.").arg(file), true);
        return false;
    }

    return true;
}

bool EpisodeListing::isDownloaded(PodcastEpisode *episode)
{
    // Get the number of times the episode url appears in the database.
    // An episode shouldn't appear in the database more than once but it could
    // happen. More than one entry is not an issue.
    if (execQuery(QString("SELECT count(*) FROM episodes WHERE url='%1';")
        .arg(episode->getUrl().toString())))
    {
        if (!m_query->next()) {
            // Something is wrong with the db. The query was successful but we
            // can't get the result.
            emit error(tr("Could not determine if episode %1 has been"
                " downloaded.").arg(episode->getUrl().toString()), true);
        }
        else {
            // If it appears 0 times than it hasn't been downloaded.
            if (m_query->value(0).toInt() == 0) {
                return false;
            }
        }
    }

    return true;
}

void EpisodeListing::setDownloaded(PodcastEpisode *episode)
{
    // Make an entry in the database for the episode url.
    if (!isDownloaded(episode)) {
        execQuery(QString("INSERT INTO episodes (url) values('%1');")
            .arg(episode->getUrl().toString()));
    }
}

bool EpisodeListing::execQuery(const QString &query)
{
    // We can't use a db that hasn't been opened.
    if (!m_db.isOpen()) {
        emit error(tr("No episode listing file has been opened."), true);
        return false;
    }

    // Execute the query setting any errors if unsuccessful.
    if (!m_query->exec(query))
    {
        emit error(tr("Database Query (%1) failed because %2.").arg(query)
            .arg(m_query->lastError().text()), false);
        return false;
    }

    return true;
}
