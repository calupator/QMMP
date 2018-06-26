/***************************************************************************
 *   Copyright (C) 2008-2017 by Ilya Kotov                                 *
 *   forkotov02@ya.ru                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <QDir>
#include <QPluginLoader>
#include <QObject>
#include <QList>
#include <QApplication>
#include <cstdlib>
#include <iostream>
#include <qmmp/qmmp.h>
#include <qmmp/soundcore.h>
#include "uihelper.h"
#include "mediaplayer.h"
#include "commandlinemanager.h"

using namespace std;

QList<CommandLineOption *> *CommandLineManager::m_options = 0;
QHash<CommandLineOption*, QString> *CommandLineManager::m_files = 0;

void CommandLineManager::checkOptions()
{
    if (!m_options)
    {
        m_options = new QList<CommandLineOption *>;
        m_files = new QHash<CommandLineOption*, QString>;

        foreach (QString filePath, Qmmp::findPlugins("CommandLineOptions"))
        {
            QPluginLoader loader(filePath);
            QObject *plugin = loader.instance();
            if (loader.isLoaded())
                /*qDebug("CommandLineManager: loaded plugin %s", qPrintable(QFileInfo(filePath).filePath()));*/;
            else
                qWarning("CommandLineManager: %s", qPrintable(loader.errorString ()));

            CommandLineOption *option = 0;
            if (plugin)
                option = qobject_cast<CommandLineOption *>(plugin);

            if (option)
            {
                m_options->append(option);
                m_files->insert(option, filePath);
                qApp->installTranslator(option->createTranslator(qApp));
            }
        }
    }
}

QString CommandLineManager::executeCommand(const QString& opt_str, const QStringList &args)
{
    checkOptions();
    if(!UiHelper::instance() || !SoundCore::instance() || !MediaPlayer::instance())
    {
        qWarning("CommandLineManager: player objects are not created");
        return QString();
    }
    foreach(CommandLineOption *opt, *m_options)
    {
        if (opt->identify(opt_str))
        {
            return opt->executeCommand(opt_str, args);
        }
    }
    return QString();
}

bool CommandLineManager::hasOption(const QString &opt_str)
{
    checkOptions();
    foreach(CommandLineOption *opt, *m_options)
    {
        if (opt->identify(opt_str))
            return true;
    }
    return false;
}

void CommandLineManager::printUsage()
{
    checkOptions();
    foreach(CommandLineOption *opt, *m_options)
    {
        foreach (QString line, opt->helpString())
        {
            QString str = formatHelpString(line);
            if(!str.isEmpty())
                cout << qPrintable(str) << endl;
        }
    }
}

QString CommandLineManager::formatHelpString(const QString &line)
{
    QStringList list = line.split("||", QString::SkipEmptyParts);
    if(list.count() == 1)
        return list.at(0);
    else if(list.count() >= 2)
        return list.at(0).leftJustified(25) + list.at(1);
    return QString();
}
