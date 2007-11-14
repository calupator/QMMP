/***************************************************************************
 *   Copyright (C) 2007 by Ilya Kotov                                      *
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

#include <QMainWindow>
#include <QApplication>
#include <QMouseEvent>
#include <QMenu>
#include <QSettings>

#include <soundcore.h>
#include "symboldisplay.h"
#include "skin.h"
#include "button.h"
#include "dock.h"
#include "titlebarcontrol.h"
#include "shadedvisual.h"
#include "titlebar.h"

TitleBar::TitleBar(QWidget *parent)
        : PixmapWidget(parent)
{
    m_align = FALSE;
    m_skin = Skin::getPointer();
    setPixmap(m_skin->getTitleBar(Skin::TITLEBAR_A));
    m_mw = qobject_cast<MainWindow*>(parent);
    m_shaded = FALSE;
    m_currentTime = 0;
    //buttons
    m_menu = new Button(this,Skin::BT_MENU_N,Skin::BT_MENU_P);
    connect(m_menu,SIGNAL(clicked()),this,SLOT(showMainMenu()));
    m_menu->move(6,3);
    m_minimize = new Button(this,Skin::BT_MINIMIZE_N,Skin::BT_MINIMIZE_P);
    m_minimize->move(244,3);
    connect(m_minimize, SIGNAL(clicked()), m_mw, SLOT(showMinimized()));
    m_shade = new Button(this,Skin::BT_SHADE1_N,Skin::BT_SHADE1_P);
    m_shade->move(254,3);
    connect(m_shade, SIGNAL(clicked()), SLOT(shade()));
    m_close = new Button(this,Skin::BT_CLOSE_N,Skin::BT_CLOSE_P);
    m_close->move(264,3);
    connect(m_close, SIGNAL(clicked()), m_mw, SLOT(handleCloseRequest()));
    setActive(FALSE);
    connect(m_skin, SIGNAL(skinChanged()), this, SLOT(updateSkin()));

    QSettings settings(QDir::homePath()+"/.qmmp/qmmprc", QSettings::IniFormat);
    if (settings.value("Display/shaded", FALSE).toBool())
        shade();
    m_align = TRUE;
}


TitleBar::~TitleBar()
{
    QSettings settings(QDir::homePath()+"/.qmmp/qmmprc", QSettings::IniFormat);
    settings.setValue("Display/shaded", m_shaded);
}

void TitleBar::mousePressEvent(QMouseEvent* event)
{
    switch ((int) event->button ())
    {
    case Qt::LeftButton:
    {
        m_pos = event->pos();
        Dock::getPointer()->calculateDistances();
        break;
    }
    case Qt::RightButton:
    {
        m_mw->menu()->exec(event->globalPos());
    }
    }
}

void TitleBar::mouseReleaseEvent(QMouseEvent*)
{
    Dock::getPointer()->updateDock();
}
void TitleBar::mouseMoveEvent(QMouseEvent* event)
{
    QPoint npos = (event->globalPos()-m_pos);
    Dock::getPointer()->move(m_mw, npos);
}

void TitleBar::setActive(bool a)
{
    if (a)
    {
        if (m_shaded)
        {
            setPixmap(m_skin->getTitleBar(Skin::TITLEBAR_SHADED_A));
            m_shade2->show();
        }
        else
        {
            setPixmap(m_skin->getTitleBar(Skin::TITLEBAR_A));
            m_shade->show();
        }
        m_menu->show();
        m_minimize->show();
        m_close->show();
    }
    else
    {
        if (m_shaded)
        {
            setPixmap(m_skin->getTitleBar(Skin::TITLEBAR_SHADED_I));
            m_shade2->hide();
        }
        else
        {
            setPixmap(m_skin->getTitleBar(Skin::TITLEBAR_I));
            m_shade->hide();
        }
        m_menu->hide();
        m_minimize->hide();
        m_close->hide();
    }
}

void TitleBar::updateSkin()
{
    setActive(FALSE);
}

void TitleBar::showMainMenu()
{
    m_mw->menu()->exec(m_menu->mapToGlobal(m_menu->pos()));
}

void TitleBar::shade()
{
    m_shaded = !m_shaded;

    if (m_shaded)
    {
        m_mw->setFixedSize(275,14);
        setPixmap(m_skin->getTitleBar(Skin::TITLEBAR_SHADED_A));
        m_shade->hide();
        m_shade2 = new Button(this,Skin::BT_SHADE2_N, Skin::BT_SHADE2_P);
        m_shade2->move(254,3);
        connect(m_shade2, SIGNAL(clicked()), SLOT(shade()));
        m_shade2->show();
        m_currentTime = new SymbolDisplay(this, 6);
        m_currentTime->move(127,4);
        m_currentTime->show();
        m_currentTime->display("--:--");
        m_control = new TitleBarControl(this);
        m_control->move(168,2);
        m_control->show();
        connect (m_control, SIGNAL (nextClicked()), parent(), SLOT (next()));
        connect (m_control, SIGNAL (previousClicked()), parent(), SLOT (previous()));
        connect (m_control, SIGNAL (playClicked()), parent(), SLOT (play()));
        connect (m_control, SIGNAL (pauseClicked()), parent(), SLOT (pause()));
        connect (m_control, SIGNAL (stopClicked()), parent(), SLOT (stop()));
        connect (m_control, SIGNAL (ejectClicked()), parent(), SLOT (addFile()));
        m_visual = new ShadedVisual(this);
        SoundCore::instance()->addVisualization(m_visual);
        m_visual->show();
        m_visual->move(79,5);
    }
    else
    {
        m_mw->setFixedSize(275,116);
        setPixmap(m_skin->getTitleBar(Skin::TITLEBAR_A));
        delete m_shade2;
        delete m_currentTime;
        delete m_control;
        SoundCore::instance()->removeVisual(m_visual);
        delete m_visual;
        m_shade2 = 0;
        m_currentTime = 0;
        m_shade->show();
    }
    if(m_align)
        Dock::getPointer()->align(m_mw, m_shaded? -102: 102);
}

QString TitleBar::formatTime ( int sec )
{
    int minutes = sec / 60;
    int seconds = sec % 60;

    QString str_minutes = QString::number ( minutes );
    QString str_seconds = QString::number ( seconds );

    if ( minutes < 10 ) str_minutes.prepend ( "0" );
    if ( seconds < 10 ) str_seconds.prepend ( "0" );

    return str_minutes + ":" + str_seconds;
}

void TitleBar::setInfo(const OutputState &st)
{
    if (!m_currentTime)
        return;
    switch ( ( int ) st.type() )
    {
    case OutputState::Info:
    {
        m_currentTime->display(formatTime(st.elapsedSeconds()));
        break;
    }
    case OutputState::Stopped:
    {
        m_currentTime->display("--:--");
        break;
    }
    }
}
