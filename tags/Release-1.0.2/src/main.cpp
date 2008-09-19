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

#include <QCoreApplication>
#include <QMetaObject>

#include "client.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setOrganizationName("nachtimwald");
    QCoreApplication::setOrganizationDomain("nachtimwald.com");
    QCoreApplication::setApplicationName("niw-podcast-downloader");

    Client client;

    // Call client's run slot when the event loop has been started. The client
    // relies on the event loop and will not work without it.
    QMetaObject::invokeMethod(&client, "run", Qt::QueuedConnection);

    return app.exec();
}
