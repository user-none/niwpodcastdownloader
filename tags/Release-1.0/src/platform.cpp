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

#include "platform.h"

// Include the necessary headers for the given platform.
#ifndef NO_PLATFORM
    #if defined(Q_OS_UNIX)
        #include <sys/statvfs.h>
    #elif defined(Q_OS_WIN32)
        #include <windows.h>
    #endif
#endif

qlonglong Platform::getFreeDiskSpace(const QString &path)
{
    qlonglong freeSpace = -1;

// if NO_PLATFORM has been defined don't include any of this.
#ifndef NO_PLATFORM
#if defined(Q_OS_UNIX)
    struct statvfs buffer;

    if (statvfs(path.toUtf8(), &buffer) == 0) {
         freeSpace = buffer.f_bavail * (buffer.f_bsize / 1024);
    }
#elif defined(Q_OS_WIN32)
    ULARGE_INTEGER free;

    if (GetDiskFreeSpaceExA(path.toUtf8(), &free, NULL, NULL) != 0) {
        freeSpace = free.QuadPart / 1024;
    }
#endif
#endif

    return freeSpace;
}

QString Platform::commandLineArgumentFlag()
{
    QString flag = "-";

#ifndef NO_PLATFORM
#if defined(Q_OS_WIN32)
    flag = "/";
#endif
#endif

    return flag;
}
