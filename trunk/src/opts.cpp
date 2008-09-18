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
#include <QTextStream>

#include "opts.h"
#include "platform.h"
#include "configure.h"

OptsOption::OptsOption(const QString &name, bool *setFlag, bool argumentRequried,
    QString *argumentSet, const QString &helpMessage,
    const QString &argumentString)
{
    m_name = name;
    m_argumentRequried = argumentRequried;
    m_helpMessage = helpMessage;
    m_argumentString = argumentString;

    set = setFlag;
    argument = argumentSet;
}

QString OptsOption::getName() const
{
    return m_name;
}

bool OptsOption::getArgumentRequired() const
{
    return m_argumentRequried;
}

QString OptsOption::getHelpMessage() const
{
    return m_helpMessage;
}

QString OptsOption::getArgumentString() const
{
    return m_argumentString;
}

bool OptsOption::operator==(const OptsOption &other) const
{
    if (getName() == other.getName()) {
        return true;
    }
    return false;
}

Opts::Opts()
{
    m_optionHelp = tr("help");
    OptsOption helpOption(m_optionHelp, 0, false, 0,
        tr("Show this help message"), "");

    m_optionVersion = tr("version");
    OptsOption versionOption(m_optionVersion, 0, false, 0,
        tr("Show the version of this application"), "");

    addOption(helpOption);
    addOption(versionOption);
}

void Opts::addOption(const OptsOption &option)
{
    if (option.getName() != "") {
        m_options.append(option);
    }
}

void Opts::parseOptions(QStringList arguments)
{
    for (int i = 0; i < arguments.size(); i++) {
        // QList.at(x) return a const string. We want to modify the string so
        // we need to set the string as a variable then modify the variable.
        // This cannot be done as a one line command. Also, we do not want
        // to use a const_cast because we do not want arguments.at(i) to be
        // modified.
        QString optName = arguments.at(i);
        optName = optName.remove(0,
            Platform::commandLineArgumentFlag().size());
        // create empty OptsOption with the name of this arugment.
        // m_options.contains must use an OptsOption. OptsOption checks if two
        // are equal by comparing name.
        OptsOption optsArg(optName, 0, false, 0, "", "");

        if (arguments.at(i).startsWith(Platform::commandLineArgumentFlag()) &&
            m_options.contains(optsArg))
        {
            int optionIndex = m_options.indexOf(optsArg);

            if (m_options.at(optionIndex).set) {
                *m_options.at(optionIndex).set = true;
            }

            if (m_options.at(optionIndex).getArgumentRequired()) {
                if (arguments.size() < i+1 && !arguments.at(i+1).startsWith(
                    Platform::commandLineArgumentFlag()))
                {
                    if (m_options.at(optionIndex).argument) {
                        *m_options.at(optionIndex).argument = arguments.at(i+1);
                    }
                    i++;
                }
                else {
                    missingArgument(m_options.at(optionIndex));
                }
            }
        }
        else {
            // Send the argument itself not optName because optName assumes
            // the option started with the commandLineArgumentFlag and removed
            // it before checking if it really is a flag.
            unknownOption(arguments.at(i));
        }
    }

    // Help and version.
    if (arguments.contains(Platform::commandLineArgumentFlag()
        + m_optionHelp))
    {
        showHelp();
    }
    else if (arguments.contains(Platform::commandLineArgumentFlag()
        + m_optionVersion))
    {
        showVersion();
    }
}

void Opts::unknownOption(const QString &option)
{
    QTextStream out(stdout);

    out << QCoreApplication::applicationName() << ": " << tr("invalid option")
        << " -- " << option << endl;
    out << tr("Try") << "`" << QCoreApplication::applicationName()
        << " " << Platform::commandLineArgumentFlag() << m_optionHelp
        << "' " << tr("for more information.") << endl;

    QCoreApplication::quit();
}


void Opts::missingArgument(const OptsOption &option)
{
    QTextStream out(stdout);

    out << QCoreApplication::applicationName() << ": " << tr("option")
        << " " << option.getName() << " " << tr("requries argument") << " "
        << option.getArgumentString() << endl;
    out << tr("Try") << "`" << QCoreApplication::applicationName()
        << " " << Platform::commandLineArgumentFlag() << m_optionHelp
        << "' " << tr("for more information.") << endl;

    QCoreApplication::quit();
}

void Opts::showHelp()
{
    QTextStream out(stdout);

    out << QCoreApplication::applicationName() << " [" << tr("Options") << "]"
        << endl;
    out << endl;

    Q_FOREACH (OptsOption option, m_options) {
        QString message;

        message += Platform::commandLineArgumentFlag() + option.getName()
            + "    ";
        if (option.getArgumentRequired() && !option.getArgumentString()
            .isEmpty())
        {
            message += "<" + option.getArgumentString() + ">    ";
        }
        out << message << endl;

        for(int i = 0; (i*75) < option.getHelpMessage().size(); i++) {
            out << "    " << option.getHelpMessage().mid(i*75, 75) << endl;
        }
    }

    QCoreApplication::quit();
}

void Opts::showVersion()
{
    QTextStream out(stdout);

    out << QCoreApplication::applicationName() << endl;
    out << "    " << tr("Version") << ": " << Configure::applicationVersion
        << endl;

    QCoreApplication::quit();
}
