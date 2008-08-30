/***************************************************************************
 *   Copyright (C) 2008 by Ilya Kotov                                      *
 *   forkotov02@hotmail.ru                                                 *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <qmmp/soundcore.h>

#include "mainwindow.h"
#include "builtincommandlineoption.h"

BuiltinCommandLineOption::BuiltinCommandLineOption(QObject *parent)
 : QObject(parent)
{
}


BuiltinCommandLineOption::~BuiltinCommandLineOption()
{
}

// BuiltinCommandLineOption methods implementation
bool BuiltinCommandLineOption::identify(const QString & str) const
{
    if (
        str == QString("--help") ||
        str == QString("--next") ||
        str == QString("--previous") ||
        str == QString("--play") ||
        str == QString("--pause") ||
        str == QString("--play-pause") ||
        str == QString("--stop") ||
        str.startsWith("--volume") ||
        str.startsWith("--jump-to-file") ||
        str.startsWith("--toggle-visibility") ||
        str.startsWith("--add-file") ||
        str.startsWith("--add-dir")
    )
    {
        return TRUE;
    }

    return FALSE;
}

const QString BuiltinCommandLineOption::helpString() const
{
    return  QString(
                "--next               "+tr("Skip forward in playlist")+ "\n" +
                "--previous           "+tr("Skip backwards in playlist")+"\n" +
                "--play               "+tr("Start playing current song")+"\n" +
                "--pause              "+tr("Pause current song")+ "\n"
                "--play-pause         "+tr("Pause if playing, play otherwise")+ "\n"
                "--stop               "+tr("Stop current song")+ "\n" +
                "--next               "+tr("Skip forward in playlist")+ "\n" +
                "--volume             "+tr("Set playback volume(example: qmmp --volume20, qmmp --volume100)")+ "\n"
                "--jump-to-file       "+tr("Display Jump to File dialog")+ "\n" +
                "--toggle-visibility  "+tr("Show/hide application")+ "\n" +
                "--add-file           "+tr("Display Add File dialog")+ "\n" +
                "--add-dir            "+tr("Display Add Directory dialog")
            );
}

void BuiltinCommandLineOption::executeCommand(const QString & option_string, MainWindow *mw)
{
    if (option_string == "--play")
    {
        mw->play();
    }
    else if (option_string == "--stop")
    {
        mw->stop();
        mw->mainDisplay()->hideTimeDisplay();
    }
    else if (option_string == "--pause")
    {
        mw->pause();
    }
    else if (option_string == "--next")
    {
        mw->next();
        if (!mw->soundCore()->isInitialized())
            mw->play();
    }
    else if (option_string == "--previous")
    {
        mw->previous();
        if (!mw->soundCore()->isInitialized())
            mw->play();
    }
    else if (option_string == "--play-pause")
    {
        mw->playPause();
    }
    else if (option_string == "--jump-to-file")
    {
        mw->jumpToFile();
    }
    else if (option_string == "--toggle-visibility")
    {
        mw->toggleVisibility();
    }
    else if (option_string == "--add-file")
    {
        mw->addFile();
    }
    else if (option_string == "--add-dir")
    {
        mw->addDir();
    }
    else if (option_string.startsWith("--volume"))
    {
        QString vol_str(option_string);
        vol_str.remove("--volume");
        bool ok = FALSE;
        int volume = vol_str.toUInt(&ok);
        if (ok)
        {
            mw->soundCore()->setVolume(volume,volume);
        }
    }
}

const QString BuiltinCommandLineOption::name() const
{
    return "BuiltinCommandLineOption";
}

