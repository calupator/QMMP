/***************************************************************************
 *   Copyright (C) 2017-2018 by Ilya Kotov                                 *
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

#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QWidget>
#include <QMutex>
#include <QImage>

class QMenu;
class SoundCore;

class VideoWindow : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWindow(QWidget *parent = nullptr);

    void addImage(const QImage &img);

signals:
    void resizeRequest(const QSize &size);
    void stopRequest();

private slots:
    void setFullScreen(bool enabled);
    void forward();
    void backward();

private:
    void paintEvent(QPaintEvent *);
    bool event(QEvent *e);
    void closeEvent(QCloseEvent *);
    void contextMenuEvent(QContextMenuEvent *event);
    QMutex m_mutex;
    QImage m_image;
    QMenu *m_menu;
    SoundCore *m_core;
};

#endif // VIDEOWINDOW_H
