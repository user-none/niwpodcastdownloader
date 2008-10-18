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

#ifndef PODCAST_H
#define PODCAST_H

#include <QList>

#include "downloaditem.h"
#include "podcastepisode.h"

/**
 * A representation of a podcast.
 *
 * Hold information relating to a podcast and a list of episodes.
 * Can parse an rss feed to generate a list of episodes.
 *
 * Note when destroyed, the podcast object will NOT delete any associated
 * PodcastEpisode objects.
 */
class Podcast : public DownloadItem
{
    Q_OBJECT

    public:
        Podcast();

        /**
         * Has the podcast been set to init mode.
         *
         * Init mode - all episodes should be marked as downloaded and not
         * actually downloaded. This is useful for adding a new podcast and
         * not downloading all back episodes.
         *
         * The podcast itself will not make any changes based on how init is
         * set. It is up to the user of the Podcast object to determine how it
         * should handle init.
         *
         * @return True if the podcast has been set to init mode.
         */
        bool isInit();
        /**
         * Should the not modified response form the server be ignored.
         *
         * This is dependant on the Last-Modified and If-Modified-Since
         * headers. Not all servers report the Last-Modified header and not
         * all servers report it correctly. Therefore this can be disabled on
         * a per podcast basis if necessary.
         *
         * @return True if the downloader should ignore the not modified
         * response from the server and verify that there are no new episodes
         * by downloading and parsing the RSS feed.
         */
        bool isIgnoreNotModified();

        /**
         * Gets the category of the podcast.
         *
         * @return The category. An empty string if no category has been set.
         */
        QString getCategory() const;

        /**
         * Gets a list of episodes.
         *
         * The rss feed associated with the podcast needs to be downloaded
         * before the episode list will be populated.
         *
         * @return The list of episodes.
         */
        QList<PodcastEpisode *> getEpisodes() const;
        /**
         * Gets the number of episodes in the episode list.
         *
         * @return The total number of episodes avaliable.
         */
        int getEpisodeCount();
        /**
         * Removes the first podcast episode and returns it.
         *
         * @return The first podcast episode in the list of episodes. 0 if the
         * episode list is empty.
         */
        PodcastEpisode* takeFirstEpisode();

        /**
         * Marks the podcast as beining in init mode.
         *
         * @param init True if the podcast should be marked as needing init.
         */
        void setInit(bool init);
        /**
         * Markst he podcast as wanting to ignore the not modified (304)
         * response from the server.
         *
         * Instead the podcast should download its RSS feed and determine if
         * there are no new episodes by parsing the feed.
         *
         * @param ignore True if the podcast should ignore the not modified
         * response.
         */
        void setIgnoreNotModified(bool ignore);

        /**
         * Sets the category of the podcast.
         *
         * @param category The category of the podcast.
         */
        void setCategory(const QString &category);

        /**
         * Remove the episode from the episode list.
         *
         * This is used in conjunction with previously downloaded episodes.
         * This will delete the episode object. It will also remove the
         * episode object from the list.
         *
         * @param *episode The episode to remove
         */
        void removeEpisode(PodcastEpisode *episode);
        /**
         * Removes all episodes after position from the episode list.
         *
         * If position is larger than the episode list no episodes are removed.
         * This will delete all episode objects removed from the list.
         *
         * @param position The position to remove from.
         */
        void truncateEpisodes(int position);
        /**
         * Removes all episode from the episode list.
         *
         * This will delete all episode objects removed from the list.
         */
        void clearEpisodeList();

    protected:
        /**
         * Parses the RSS associated with the podcast and creates a list of
         * episodes.
         *
         * This will clear the current episode list and delete all episode
         * objects. Do no call this slot if episodes in the list are being
         * used elsewhere. This only matters if the episode is referenced by
         * podcast's episode list. If the episode has been removed from the
         * list this will have no effect on the episode.
         *
         * If the episode list has been modifed by other functions this will
         * clear those changes.
         *
         * @return True on success. False if there was an error.
         *
         * @see removeEpisode
         * @see truncateEpisodes
         */
        bool downloadSuccessful();

    private:
        /**
         * The podcast's category.
         *
         * This can be empty.
         */
        QString m_category;
        /**
         * The list of episode associated with the podcast.
         */
        QList<PodcastEpisode *> m_episodes;
        /**
         * Init mode.
         */
        bool m_init;
        /**
         * Do not use the Last-Modified time when determining if their are new
         * episodes.
         */
        bool m_ignoreNotModified;
};

#endif /* PODCAST_H */
