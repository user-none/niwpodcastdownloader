#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QStringList>
#include <QUuid>

#include "database.h"

// The id is set to the application's internal name. However, anything could
// have been used as long as it's unique to this application in some way.
const QString Database::dbID = "niwpodcastdownloader";
const int Database::dbVersion = 1;

Database::Database()
{
    m_db = QSqlDatabase();
    m_dbName = "";
    m_query = 0;
}

Database::~Database()
{
    if (m_query) {
        delete m_query;
    }
    m_query = 0;

    m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_dbName);
}

bool Database::open(const QString &file)
{
    // If the db doesn't exit we will need to create the default tables later.
    // We need to mark if the file is a new file because we may need to make
    // the containing directory and run some checks before giving the file name
    // to the db object. However, the db object will make an empty file if it
    // does not exist but we need to know when it's a new file so the default
    // tables can be written.
    bool newFile = false;

    QFileInfo fileInfo(file);

    // The db cannot be a directory.
    if (fileInfo.isDir()) {
        m_openError = tr("Cannot open file %1 because it is a directory.")
            .arg(file);
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
        m_openError = tr("Cannot open file %1 because %2.").arg(file)
            .arg(m_db.lastError().text());
        return false;
    }

    // Associate a new query obecjt with the database.
    if (m_query) {
        delete m_query;
    }
    m_query = new QSqlQuery(m_db);

    if (newFile) {
        if (!createDefaultDb()) {
            return false;
        }
    }
    else {
        if (!verifyDb()) {
            return false;
        }
    }

    return true;
}

QString Database::openError()
{
    return m_openError;
}

bool Database::isDownloaded(PodcastEpisode *episode)
{
    // Get the number of times the episode url appears in the database.
    // An episode shouldn't appear in the database more than once but it could
    // happen. More than one entry is not an issue.
    if (execQuery(QString("SELECT count(*) FROM episodes WHERE url='%1';")
        .arg(episode->getUrl().toString())))
    {
        if (!m_query->next()) {
            // Something is wrong with the db. The query was successful but we
            // can't get the result. There should always be a result even when
            // it's 0.
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

void Database::setDownloaded(PodcastEpisode *episode)
{
    // Make an entry in the database for the episode url.
    if (!isDownloaded(episode)) {
        execQuery(QString("INSERT INTO episodes (url) VALUES('%1');")
            .arg(episode->getUrl().toString()));
    }
}

QString Database::getLastModified(Podcast *podcast)
{
    execQuery(QString(
        "SELECT lastmodified FROM rss WHERE url='%1';")
        .arg(podcast->getUrl().toString()));

    if (m_query->next()) {
        return m_query->value(0).toString();
    }
    else {
        return QString();
    }
}

void Database::setLastModified(Podcast *podcast)
{
    // Check if there is already an entry for the podcast.
    execQuery(QString("SELECT ROWID from rss where url='%1';")
        .arg(podcast->getUrl().toString()));

    if (m_query->next()) {
        // Update the modified time.
        execQuery(QString(
            "UPDATE rss SET lastmodified='%1' WHERE ROWID='%2';")
            .arg(podcast->getLastModified())
            .arg(m_query->value(0).toString()));
    }
    else {
        // Create a new entry for the podcast.
        execQuery(QString(
            "INSERT INTO rss (url, lastmodified) VALUES('%1', '%2');")
            .arg(podcast->getUrl().toString())
            .arg(podcast->getLastModified()));
    }
}

bool Database::createDefaultDb()
{
    QStringList createQuery;

    createQuery
        << "CREATE TABLE info (key TEXT, value TEXT);"
        << "CREATE TABLE episodes (url TEXT);"
        << "CREATE TABLE rss (url TEXT, lastmodified TEXT);"
        << QString("INSERT INTO info (key, value) VALUES('id', '%1');")
            .arg(dbID)
        << QString("INSERT INTO info (key, value) VALUES('version', '%2');")
            .arg(dbVersion);

    Q_FOREACH(QString query, createQuery) {
        if (!m_query->exec(query)) {
            m_openError = tr("Could not create default database because %1.")
                .arg(m_query->lastError().text());
            return false;
        }
    }

    return true;
}

bool Database::verifyDb()
{
    QString errorPrefix = tr("Database not valid because");
    QString id = "";
    int version = 0;

    // Check for the info table.
    if (!m_query->exec("SELECT count(*) FROM info;")) {
        m_openError = tr("%1 info table not found.").arg(errorPrefix);
        return false;
    }

    // Check the db id.
    if (!m_query->exec("SELECT value FROM info WHERE key='id';")) {
        m_openError = tr("%1 id value not found.").arg(errorPrefix);
        return false;
    }
    if (m_query->next()) {
        id = m_query->value(0).toString();
    }
    if (id != dbID) {
        m_openError = tr("%1 id: %2 is not valid.").arg(errorPrefix).arg(id);
        return false;
    }

    // Check the db version.
    if (!m_query->exec("SELECT value FROM info WHERE key='version';")) {
        m_openError = tr("%1 version value not found.").arg(errorPrefix);
        return false;
    }
    if (m_query->next()) {
        version = m_query->value(0).toInt();
    }
    // Update to the lasted db format but the oldest format is verison 1.
    if (version < dbVersion && version >= 1) {
        if (!updateDb(version)) {
            return false;
        }
    }
    else if (version > dbVersion) {
        m_openError = tr("%1 database version to new. Application only"
            " supports database version <= %2.").arg(errorPrefix)
            .arg(dbVersion);
        return false;
    }

    // Database is valid.
    return true;
}

bool Database::updateDb(int version)
{
    Q_UNUSED(version);
    return true;
}

bool Database::execQuery(const QString &query)
{
    // We can't use a db that hasn't been opened.
    if (!m_db.isOpen()) {
        emit error(tr("Database not open."), true);
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
