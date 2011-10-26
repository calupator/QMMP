/***************************************************************************
 *   Copyright (C) 2008-2011 by Ilya Kotov                                 *
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
#include "playlistitem.h"
#include "mediaplayer.h"

#define MAX_ERRORS 4

MediaPlayer *MediaPlayer::m_instance = 0;

MediaPlayer::MediaPlayer(QObject *parent)
        : QObject(parent)
{
    if(m_instance)
        qFatal("StateHandler: only one instance is allowed");
    m_instance = this;
    m_pl_manager = 0;
    m_core = 0;
    m_skips = 0;
    m_repeat = false;
    m_noPlaylistAdvance = false;
    QTranslator *translator = new QTranslator(parent);
    QString locale = Qmmp::systemLanguageID();
    translator->load(QString(":/libqmmpui_") + locale);
    qApp->installTranslator(translator);
    m_core = new SoundCore(this);
    m_pl_manager = new PlayListManager(this);
    connect(m_core, SIGNAL(nextTrackRequest()), SLOT(updateNextUrl()));
    connect(m_core, SIGNAL(finished()), SLOT(playNext()));
    connect(m_core, SIGNAL(stateChanged(Qmmp::State)), SLOT(processState(Qmmp::State)));
    connect(m_core, SIGNAL(metaDataChanged()),SLOT(showMetaData()));
}

MediaPlayer::~MediaPlayer()
{
    m_instance = 0;
}

MediaPlayer* MediaPlayer::instance()
{
    return m_instance;
}

PlayListManager *MediaPlayer::playListManager()
{
    return m_pl_manager;
}

bool MediaPlayer::isRepeatable() const
{
    return m_repeat;
}

bool MediaPlayer::isNoPlaylistAdvance() const
{
    return m_noPlaylistAdvance;
}

void MediaPlayer::play(qint64 offset)
{
    m_pl_manager->currentPlayList()->doCurrentVisibleRequest();
    if (m_core->state() == Qmmp::Paused)
    {
        m_core->pause();
        return;
    }

    if (m_pl_manager->currentPlayList()->count() == 0)
        return;

    QString s = m_pl_manager->currentPlayList()->currentItem()->url();
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
    m_core->play(s, false, offset);
}

void MediaPlayer::stop()
{
    m_core->stop();
    m_nextUrl.clear();
}

void MediaPlayer::next()
{
    bool playNext = m_core->state() != Qmmp::Stopped;
    stop();
    if (m_pl_manager->currentPlayList()->next() && playNext)
        play();
}

void MediaPlayer::previous()
{
    bool playNext = m_core->state() != Qmmp::Stopped;
    stop();
    if (m_pl_manager->currentPlayList()->previous() && playNext)
        play();
}

void MediaPlayer::setRepeatable(bool r)
{
    if (r != m_repeat)
    {
        if(r)
        {
            disconnect(m_core, SIGNAL(finished()), this, SLOT(playNext()));
            connect(m_core, SIGNAL(finished()), SLOT(play()));
        }
        else
        {
            disconnect(m_core, SIGNAL(finished()), this, SLOT(play()));
            connect(m_core, SIGNAL(finished()), SLOT(playNext()));
        }
        m_repeat = r;
        emit repeatableChanged(r);
    }
}

void MediaPlayer::playNext()
{
    if(m_noPlaylistAdvance)
    {
        stop();
        return;
    }
    if (!m_pl_manager->currentPlayList()->next())
    {
        stop();
        return;
    }
    play();
}

void MediaPlayer::setNoPlaylistAdvance(bool enabled)
{
    if (enabled != m_noPlaylistAdvance)
    {
        m_noPlaylistAdvance = enabled;
        emit noPlaylistAdvanceChanged(enabled);
    }
}

void MediaPlayer::updateNextUrl()
{
    m_nextUrl.clear();
    PlayListItem *item = 0;
    if(isRepeatable())
        item = m_pl_manager->currentPlayList()->currentItem();
    else if(!m_noPlaylistAdvance)
        item = m_pl_manager->currentPlayList()->nextItem();

    if(item)
    {
        bool ok = m_core->play(item->url(), true);
        if(ok)
        {
            m_nextUrl = item->url();
            qDebug("MediaPlayer: next track state: received");
        }
        else
            qDebug("MediaPlayer: next track state: error");
    }
    else
        qDebug("MediaPlayer: next track state: unknown");

}

void MediaPlayer::processState(Qmmp::State state)
{
    switch ((int) state)
    {
    case Qmmp::NormalError:
        stop();
        if (m_skips <= MAX_ERRORS)
        {
            m_skips++;
            playNext();
        }
        break;
    case Qmmp::FatalError:
        stop();
        break;
    case Qmmp::Playing:
        m_skips = 0;
    default:
        ;
    }
}

void MediaPlayer::showMetaData()
{
    qDebug("===== metadata ======");
    qDebug("ARTIST = %s", qPrintable(m_core->metaData(Qmmp::ARTIST)));
    qDebug("TITLE = %s", qPrintable(m_core->metaData(Qmmp::TITLE)));
    qDebug("ALBUM = %s", qPrintable(m_core->metaData(Qmmp::ALBUM)));
    qDebug("COMMENT = %s", qPrintable(m_core->metaData(Qmmp::COMMENT)));
    qDebug("GENRE = %s", qPrintable(m_core->metaData(Qmmp::GENRE)));
    qDebug("YEAR = %s", qPrintable(m_core->metaData(Qmmp::YEAR)));
    qDebug("TRACK = %s", qPrintable(m_core->metaData(Qmmp::TRACK)));
    qDebug("== end of metadata ==");

    PlayListModel *pl = m_pl_manager->currentPlayList();
    if (pl->currentItem() && pl->currentItem()->url() == m_core->metaData().value(Qmmp::URL))
    {
        pl->currentItem()->updateMetaData(m_core->metaData());
        pl->doCurrentVisibleRequest();
    }
}
