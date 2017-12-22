/*  smplayer, GUI front-end for mplayer.
    Copyright (C) 2006-2014 Ricardo Villalba <rvm@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


    Winfileassoc.h
    Handles file associations in Windows
    Author: Florin Braghis (florin@libertv.ro)
*/

#ifndef WINFILEASSOC_H
#define WINFILEASSOC_H

#include <QString>
#include <QStringList>
#include "qmmpui_export.h"

/**
    @internal
*/
class QMMPUI_EXPORT WinFileAssoc
{
protected:
    QString m_ClassId; //Application ClassId
    QString m_ClassId2; //The ClassId created by explorer if user selects 'Open With...', usually qmmp.exe
    QString m_AppName;

protected:
    bool CreateClassId(const QString& executablePath, const QString& friendlyName);
    bool VistaGetDefaultApps(const QStringList &extensions, QStringList& registeredExt);
    int  VistaSetAppsAsDefault(const QStringList& extensions);
public:
    WinFileAssoc(const QString ClassId = "QmmpFileAudio", const QString AppName="Qmmp");
    //Checks the registry to see which extensions are registered with SMPlayer
    bool GetRegisteredExtensions(const QStringList& extensionsToCheck, QStringList& registeredExtensions);
    bool RemoveClassId();
    int CreateFileAssociations(const QStringList& fileExtensions);
    int RestoreFileAssociations(const QStringList& fileExtensions);
};

#endif
