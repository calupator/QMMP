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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef PROJECTMWIDGET_H
#define PROJECTMWIDGET_H

#include <QGLWidget>


class QMenu;
class projectM;


/**
    @author Ilya Kotov <forkotov02@hotmail.ru>
*/
class ProjectMWidget : public QGLWidget
{
    Q_OBJECT
public:
    ProjectMWidget(QWidget *parent = 0);

    ~ProjectMWidget();

    projectM *projectMInstance();

protected:
    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    virtual void paintGL();
    virtual void mousePressEvent (QMouseEvent *event);

private slots:
    void showHelp();
    void showPresetName();
    void showTitle();
    void nextPreset();
    void previousPreset();
    void randomPreset();
    void lockPreset();
    void fullScreen();
    void updateTitle();

private:
    projectM *m_projectM;
    QMenu *m_menu;
    void createActions();


};

#endif
