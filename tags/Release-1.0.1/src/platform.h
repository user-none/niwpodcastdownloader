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

#ifndef PLATFORM_H
#define PLATFORM_H

#include <QtGlobal>
#include <QString>

/**
 * Platform specific functions.
 *
 * The functions in this clas implement platform specific functionalitiy. This
 * can be disabled at build time. All functions in this class will return an
 * unimplemented value making the functions safe to be called/used on platforms
 * that do not have platform specific code.
 */
class Platform
{
    public:
        /**
         * Gets the amount of free disk space referenced by path.
         *
         * @return The amount of free disk space in KB. -1 if getting the
         * free space is not supported on the platform.
         */
        static qlonglong getFreeDiskSpace(const QString &path);
        /**
         * Gets the platform specific command line argument prefix.
         *
         * On Windows this is defined as /. All other platforms it is defined
         * as -.
         *
         * @return The flag prefix.
         */
        static QString commandLineArgumentFlag();
};

#endif /* PLATFORM_H */
