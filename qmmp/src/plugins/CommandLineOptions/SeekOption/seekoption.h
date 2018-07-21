/***************************************************************************
 *   Copyright (C) 2010-2017 by Ilya Kotov                                 *
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

#ifndef SEEKOPTION_H
#define SEEKOPTION_H

#include <QString>
#include <QObject>
#include <QStringList>
#include <qmmpui/commandlineoption.h>
#include <qmmpui/commandlinemanager.h>


class SeekOption : public QObject, public CommandLineOption
{
Q_OBJECT
Q_PLUGIN_METADATA(IID "org.qmmp.qmmpui.CommandLineOptionInterface.1.0")
Q_INTERFACES(CommandLineOption)
public:
    virtual bool identify(const QString& opt_str) const;
    virtual const QString name() const;
    virtual const QStringList helpString() const;
    virtual QString executeCommand(const QString& opt_str, const QStringList &args);
};

#endif
