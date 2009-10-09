/***************************************************************************
 *   Copyright (C) 2008-2009 by Ilya Kotov                                 *
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

#include <QApplication>
#include <QString>
#include <QTranslator>
#include <QLocale>

#include <qmmp/soundcore.h>
#include "playlistmodel.h"
#include "playlistitem.h"

#include "mediaplayer.h"

#define MAX_SKIPS 5

MediaPlayer *MediaPlayer::m_instance = 0;

MediaPlayer::MediaPlayer(QObject *parent)
        : QObject(parent)
{
    m_instance = this;
    m_model = 0;
    m_core = 0;
    m_skips = 0;
    m_repeat = FALSE;
    QTranslator *translator = new QTranslator(parent);
    QString locale = Qmmp::systemLanguageID();
    translator->load(QString(":/libqmmpui_") + locale);
    qApp->installTranslator(translator);
}


MediaPlayer::~MediaPlayer()
{}

MediaPlayer* MediaPlayer::instance()
{
    return m_instance;
}

void MediaPlayer::initialize(SoundCore *core, PlayListModel *model)
{
    Q_CHECK_PTR(core);
    Q_CHECK_PTR(model);
    m_core = core;
    m_model = model;
    m_repeat = FALSE;
    connect(m_core, SIGNAL(aboutToFinish()), SLOT(updateNextUrl()));
    connect(m_core, SIGNAL(finished()), SLOT(next()));
}

PlayListModel *MediaPlayer::playListModel()
{
    return m_model;
}

bool MediaPlayer::isRepeatable() const
{
    return m_repeat;
}

void MediaPlayer::play()
{
    m_model->doCurrentVisibleRequest();
    if (m_core->state() == Qmmp::Paused)
    {
        m_core->pause();
        return;
    }

    if (m_model->count() == 0)
        return;

    QString s = m_model->currentItem()->url();
    if (s.isEmpty())
    {
        m_nextUrl.clear();
        return;
    }
    if(m_nextUrl == s)
    {
        m_nextUrl.clear();
        return;
    }

    if (!m_core->play(s))
    {
        //find out the reason why playback failed
        switch ((int) m_core->state())
        {
        case Qmmp::FatalError:
        {
            stop();
            return; //unrecovable error in output, so abort playing
        }
        case Qmmp::NormalError:
        {
            //error in decoder, so we should try to play next song
            m_skips++;
            if (m_skips > MAX_SKIPS)
            {
                stop();
                qWarning("MediaPlayer: skip limit exceeded");
                break;
            }
            qApp->processEvents();
            if (!m_model->isEmptyQueue())
            {
                m_model->setCurrentToQueued();
            }
            else if (!m_model->next())
            {
                stop();
                return;
            }
            play();
            break;
        }
        }
    }
    else
        m_skips = 0;
}

void MediaPlayer::stop()
{
    m_core->stop();
    m_nextUrl.clear();
}

void MediaPlayer::next()
{
    if (!m_model->isEmptyQueue())
    {
        m_model->setCurrentToQueued();
    }
    else if (!m_model->next())
    {
        stop();
        return;
    }

    if (m_core->state() != Qmmp::Stopped)
    {
        if (m_core->state() == Qmmp::Paused)
            stop();
        play();
    }
}

void MediaPlayer::previous()
{
    if (!m_model->previous())
    {
        stop();
        return;
    }

    if (m_core->state() != Qmmp::Stopped)
    {
        if (m_core->state() == Qmmp::Paused)
            stop();
        play();
    }
}

void MediaPlayer::setRepeatable(bool r)
{
    if (r != m_repeat && !r)
    {
        disconnect(m_core, SIGNAL(finished()), this, SLOT(play()));
        connect(m_core, SIGNAL(finished()), SLOT(next()));
    }
    else if (r != m_repeat && r)
    {
        disconnect(m_core, SIGNAL(finished()), this, SLOT(next()));
        connect(m_core, SIGNAL(finished()), SLOT(play()));
    }
    m_repeat = r;
    emit repeatableChanged(r);
}

void MediaPlayer::updateNextUrl()
{
    if(m_model->nextItem() && !isRepeatable())
    {
        m_core->play(m_model->nextItem()->url(), TRUE);
        m_nextUrl = m_model->nextItem()->url();
        qDebug("next url");
    }
    else
        m_nextUrl.clear();
}
