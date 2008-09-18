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

#ifndef OPTS_H
#define OPTS_H

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

/**
 * Defines an option used by Opts.
 *
 * This defines the name of the options, as well as help strings.
 */
class OptsOption
{
    public:
        OptsOption(const QString &name, bool *setFlag, bool argumentRequried,
            QString *argumentSet, const QString &helpMessage,
            const QString &argumentString);

        /**
         * Gets the name of the option.
         *
         * @return The name of the option.
         */
        QString getName() const;
        /**
         * Whether the argument is required.
         *
         * @return True if the option requires an argument.
         */
        bool getArgumentRequired() const;
        /**
         * The help message.
         *
         * @return The help message.
         */
        QString getHelpMessage() const;
        /**
         * The argument to show in the help message.
         *
         * @return The name of the argument to show in the help message.
         */
        QString getArgumentString() const;

        /**
         * Whether the option was given on the command line.
         */
        bool *set;
        /**
         * The argument.
         *
         * This is only set if argumentRequried is true;
         */
        QString *argument;

        bool operator==(const OptsOption &other) const;

    private:
        /**
         * The name of the option.
         *
         * This should not be defined with the flag prefix (/, -). The prefix
         * will be determined by Platform::commandLineArgumentFlag().
         */
        QString m_name;
        /**
         * Whether the option has a required argument.
         */
        bool m_argumentRequried;
        /**
         * The help message to display to the user when help is called.
         */
        QString m_helpMessage;
        /**
         * The name of the argument to show in the help message.
         */
        QString m_argumentString;
};

/**
 * Parses command line arguments and checks them against defined options.
 *
 * This will create default help and version options.
 *
 * Usage:
 *   QCoreApplication app(argc, argv);
 *
 *   bool optionInitSet = false;
 *   bool optionConfigSet = false;
 *   QString optionConfigArg = "";
 *
 *   OptsOption optionInit("init", &optionInitSet, false, 0, "Init mode", "");
 *   OptsOption optionConfig("config", &optionConfigSet, true,
 *     &optionConfigArg, "Specify a configuration file", "FILE");
 *
 *   Opts opts;
 *   opts.addOption(optionInit);
 *   opts.addOption(optionConfig);
 *
 *   QStringList arguments = app.arguments();
 *   arguments.removeAt(0);
 *
 *   opts.parseOptions(arguments);
 *
 *   if (optionInitSet) {
 *     qDebug() << "init mode has been set";
 *   }
 *   if (optionConfigSet) {
 *     qDebug() << "Config file: " << optionConfigArg;
 *   }
 */
class Opts : private QObject
{
    public:
        Opts();

        /**
         * Add an option to the option list.
         *
         * @param option The option to add.
         */
        void addOption(const OptsOption &option);
        /**
         * Parse the command line options.
         *
         * @param arguments A list of arguments. This list should not start
         * with the program name.
         */
        void parseOptions(QStringList arguments);

    private:
        /**
         * Error message displayed when there is an unknown option.
         *
         * This will cause the application to exit.
         *
         * @param option The option.
         */
        void unknownOption(const QString &option);
        /**
         * Error message displayed when the options argument is missing.
         *
         * This will cause the application to exit.
         *
         * @param option The option that is missing it's argument.
         */
        void missingArgument(const OptsOption &option);
        /**
         * Show the help message and exit.
         *
         * The help message is auto-generated from all options.
         */
        void showHelp();
        /**
         * Show the version.
         */
        void showVersion();

        /**
         * List of all options.
         */
        QList<OptsOption> m_options;
        /**
         * Default help option.
         */
        QString m_optionHelp;
        /**
         * Default version option.
         */
        QString m_optionVersion;
};

#endif /* OPTS_H */
