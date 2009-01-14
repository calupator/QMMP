/***************************************************************************
 *   Copyright (C) 2009 by Ilya Kotov                                      *
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
#ifndef LYRICSWINDOW_H
#define LYRICSWINDOW_H

#include <QWidget>

#include "ui_lyricswindow.h"

class QHttp;

/**
	@author Ilya Kotov <forkotov02@hotmail.ru>
*/
class LyricsWindow : public QWidget
{
Q_OBJECT
public:
    LyricsWindow(const QString &artist, const QString &title, QWidget *parent = 0);

    ~LyricsWindow();

private slots:
    void showText(bool error);
    void showState(int state);

private:
    Ui::LyricsWindow ui;
    QHttp *m_http;

};

#endif
