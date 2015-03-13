A headless podcast downloader.


# News #

  * 1.1 Released!
  * 1.1 comes with a number of new features, a new database format and a few bug fixes.
  * See CHANGELOG for a full list of changes since the last release.

Note the 1.1 release uses a new database format. 1.0.x databases are not compatible. It is recommended to delete the episodes.db file and run using the -init option to re-mark episodes as downloaded.


# Features #

  * Use the If-Modified-Since header to only download RSS feeds that have changed since the last full download.
  * Use of the If-Modified-Since header can be disabled on a global or per podcast basis if necessary.
  * Optionally can ignore downloading episodes marked as explicit with the 

&lt;itunes:explicit&gt;

 tag.
  * Tries to set the correct file name when dealing with redirects. For example http://somepodcastsite.com/redirect.mp3?http://realsite.com/realpodcast.mp3 will be saved as realpodcast.mp3 not redirect.mp3.
  * Does not have a GUI. Runs as a background task. Suitable for headless servers.
  * Uses a user configurable number of download threads to download in parallel.
  * Has the ability to check the amount of free disk space and only download if there is more space free than a user set minimum.
  * Can either download all new episodes or only a maximum number of new episodes.