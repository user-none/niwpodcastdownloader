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

#ifndef PODCASTEPISODE_H
#define PODCASTEPISODE_H

#include <QDateTime>
#include <QFile>

#include "downloaditem.h"

/**
 * A representation of a podcast episode.
 */
class PodcastEpisode : public DownloadItem
{
    Q_OBJECT

    public:
        PodcastEpisode();
        ~PodcastEpisode();

        /**
         * Get's the date the episode was published.
         *
         * @return The publish date.
         */
        QDateTime getPublishDate() const;
        /**
         * Get's the location where the episode will be downloaded to.
         *
         * @return The save location.
         */
         QString getSaveLocation() const;
        /**
         * Does this episode contain explicit content.
         *
         * Explicit content includes for example, foul language or mature
         * topics. The explicit status is determined by the itunes:explicit
         * tag in the podcast rss feed. Episodes with out the itunes:explicit
         * tag are set as not explicit.
         *
         * @return True if the podcast is explicit. False if it is not.
         */
        bool isExplicit();

        /**
         * Set the date the episode was published.
         *
         * @param date The published date.
         */
        void setPublishDate(const QDateTime &date);
        /**
         * Set where the episode should be saved on disk to.
         *
         * @param fileName The name of the file including path to save to.
         */
        void setSaveLocation(const QString &fileName);
        /**
         * Sets the explicit status of the episode.
         *
         * @param isExplicit Whether the episode contains explicit content.
         */
        void setExplicit(bool isExplicit);

        /**
         * Set the write to take place at the beginning of the file.
         *
         * This is for use when the content has moved and the download is
         * restarted.
         */
        void resetWrite();

        void setNetworkReply(QNetworkReply *reply);

        /**
         * Compares the publish date of two PodcastEpisodes to see which one
         * is more recent.
         *
         * @param ep1 The first episode to use for comparison.
         * @param ep2 The second episode to use for comparison.
         *
         * @return True if ep1 is more recent than ep2.
         */
        static bool greaterThan(PodcastEpisode *ep1, PodcastEpisode *ep2);

    public slots:
        /**
         * Write the downloaded data to disk.
         */
        void writeData();

    protected:
        bool downloadSuccessful();
        void cleanDownload();

    private:
        /**
         * When the episode was published.
         */
        QDateTime m_publishDate;

        /**
         * The file name with path to save to.
         */
        QString m_fileName;
        /**
         * The file object to use for writing.
         */
        QFile *m_file;
        /**
         * The explicit status of the episode. Episodes default to not
         * explicit.
         */
        bool m_explicit;
};

#endif /* PODCASTEPISODE_H */
