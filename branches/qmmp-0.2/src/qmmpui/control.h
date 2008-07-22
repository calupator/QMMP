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
#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>

/**
	@author Ilya Kotov <forkotov02@hotmail.ru>
*/
class Control : public QObject
{
Q_OBJECT
public:
    Control(QObject *parent = 0);

    ~Control();

    enum Command
    {
        Play = 0,
        Stop,
        Pause,
        Previous,
        Next,
        Exit,
        ToggleVisibility
    };

signals:
    void commandCalled(uint command);
    void seekCalled(int pos);
    void volumeChanged(int left, int right);

public slots:
    void play();
    void pause();
    void stop();
    void next();
    void previous();
    void seek(int pos);
    void exit();
    void toggleVisibility();
    void setVolume(int left, int right);

};

#endif
