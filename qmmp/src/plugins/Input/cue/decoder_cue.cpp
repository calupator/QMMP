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

#include <qmmp/buffer.h>
#include <qmmp/output.h>
#include <qmmp/recycler.h>
#include <qmmp/fileinfo.h>
#include <qmmp/decoderfactory.h>
#include <qmmp/soundcore.h>

#include <QObject>
#include <QFile>

#include "cueparser.h"
#include "decoder_cue.h"


DecoderCUE::DecoderCUE(QObject *parent, DecoderFactory *d, const QString &url)
        : Decoder(parent, d)
{
    path = url;
    m_decoder = 0;
    m_output2 = 0;
    m_input2 = 0;
    for (int i = 1; i < 10; ++i)
        m_bands2[i] = 0;
    m_preamp2 = 0;
    m_useEQ2 = FALSE;
}

DecoderCUE::~DecoderCUE()
{}

bool DecoderCUE::initialize()
{
    m_input2 = 0;

    QString p = QUrl(path).path();
    p.replace(QString(QUrl::toPercentEncoding("#")), "#");
    p.replace(QString(QUrl::toPercentEncoding("%")), "%");
    CUEParser parser(p);
    if (parser.count() == 0)
    {
        qWarning("DecoderCUE: invalid cue file");
        return FALSE;
    }
    int track = path.section("#", -1).toInt();
    path = parser.filePath(track);
    // find next track
    if(track <= parser.count() - 1)
        m_nextUrl = parser.info(track + 1)->path();
    //is it track of another file?
    if(QUrl(m_nextUrl).path() != p)
        m_nextUrl.clear();

    if (!QFile::exists(path))
    {
        qWarning("DecoderCUE: file \"%s\" doesn't exist", qPrintable(path));
        return FALSE;
    }
    DecoderFactory *df = Decoder::findByPath(path);
    if (!df)
    {
        qWarning("DecoderCUE: unsupported file format");
        return FALSE;
    }
    if (!df->properties().noInput)
    {
        m_input2 = new QFile(path);
        if (!m_input2->open(QIODevice::ReadOnly))
        {
            qDebug("DecoderCUE: cannot open input");
            stop();
            return FALSE;
        }
    }
    if (df->properties().noOutput)
    {
        qWarning("DecoderCUE: unsupported file format");
        return FALSE;
    }
    m_output2 = Output::create(this);
    if (!m_output2)
    {
        qWarning("DecoderCUE: unable to create output");
        return FALSE;
    }
    if (!m_output2->initialize())
    {
        qWarning("SoundCore: unable to initialize output");
        delete m_output2;
        m_output2 = 0;
        return FALSE;
    }

    m_length = parser.length(track);
    m_offset = parser.offset(track);
    m_decoder = df->create(this, m_input2, m_output2, path);
    m_decoder->setEQ(m_bands2, m_preamp2);
    m_decoder->setEQEnabled(m_useEQ2);
    connect(m_decoder, SIGNAL(playbackFinished()), SLOT(finish()));
    //replace default state handler to ignore metadata
    m_decoder->setStateHandler(new CUEStateHandler(m_decoder));
    m_output2->setStateHandler(m_decoder->stateHandler());
    connect(stateHandler(), SIGNAL(aboutToFinish()), SLOT(proccessFinish()));
    //prepare decoder and ouput objects
    m_decoder->initialize();
    qDebug("%lld == %lld", m_offset, m_length);
    m_decoder->setFragment(m_offset, m_length);
    //send metadata
    QMap<Qmmp::MetaData, QString> metaData = parser.info(track)->metaData();
    stateHandler()->dispatch(metaData);
    return TRUE;
}

qint64 DecoderCUE::totalTime()
{
    return m_decoder ? m_length : 0;
}

void DecoderCUE::seek(qint64 pos)
{
    if (m_output2 && m_output2->isRunning())
    {
        m_output2->mutex()->lock ();
        m_output2->seek(pos);
        m_output2->mutex()->unlock();
        if (m_decoder && m_decoder->isRunning())
        {
            m_decoder->mutex()->lock ();
            m_decoder->seek(pos);
            m_decoder->mutex()->unlock();
        }
    }
}

void DecoderCUE::stop()
{
    if (m_decoder /*&& m_decoder->isRunning()*/)
    {
        m_decoder->mutex()->lock ();
        m_decoder->stop();
        m_decoder->mutex()->unlock();
        //m_decoder->stateHandler()->dispatch(Qmmp::Stopped);
    }
    if (m_output2)
    {
        m_output2->mutex()->lock ();
        m_output2->stop();
        m_output2->mutex()->unlock();
    }

    // wake up threads
    if (m_decoder)
    {
        m_decoder->mutex()->lock ();
        m_decoder->cond()->wakeAll();
        m_decoder->mutex()->unlock();
    }
    if (m_output2)
    {
        m_output2->recycler()->mutex()->lock ();
        m_output2->recycler()->cond()->wakeAll();
        m_output2->recycler()->mutex()->unlock();
    }
    if (m_decoder)
        m_decoder->wait();
    if (m_output2)
        m_output2->wait();

    if (m_input2)
    {
        m_input2->deleteLater();
        m_input2 = 0;
    }
}

void DecoderCUE::pause()
{
    if (m_output2)
    {
        m_output2->mutex()->lock ();
        m_output2->pause();
        m_output2->mutex()->unlock();
    }
    else if (m_decoder)
    {
        m_decoder->mutex()->lock ();
        m_decoder->pause();
        m_decoder->mutex()->unlock();
    }

    // wake up threads
    if (m_decoder)
    {
        m_decoder->mutex()->lock ();
        m_decoder->cond()->wakeAll();
        m_decoder->mutex()->unlock();
    }

    if (m_output2)
    {
        m_output2->recycler()->mutex()->lock ();
        m_output2->recycler()->cond()->wakeAll();
        m_output2->recycler()->mutex()->unlock();
    }
}

void DecoderCUE::setEQ(double bands[10], double preamp)
{
    for (int i = 0; i < 10; ++i)
        m_bands2[i] = bands[i];
    m_preamp2 = preamp;
    if (m_decoder)
    {
        m_decoder->mutex()->lock ();
        m_decoder->setEQ(m_bands2, m_preamp2);
        m_decoder->setEQEnabled(m_useEQ2);
        m_decoder->mutex()->unlock();
    }
}

void DecoderCUE::setEQEnabled(bool on)
{
    m_useEQ2 = on;
    if (m_decoder)
    {
        m_decoder->mutex()->lock ();
        m_decoder->setEQ(m_bands2, m_preamp2);
        m_decoder->setEQEnabled(on);
        m_decoder->mutex()->unlock();
    }
}

void DecoderCUE::run()
{
    m_decoder->start();
    if (m_output2)
        m_output2->start();
}

void DecoderCUE::proccessFinish()
{
    if(nextUrlRequest(m_nextUrl))
    {
        qDebug("DecoderCUE: going to next track");
        int track = m_nextUrl.section("#", -1).toInt();
        QString p = QUrl(m_nextUrl).path();
        p.replace(QString(QUrl::toPercentEncoding("#")), "#");
        p.replace(QString(QUrl::toPercentEncoding("%")), "%");
        //update current fragment
        CUEParser parser(p);
        m_length = parser.length(track);
        m_offset = parser.offset(track);
        m_decoder->mutex()->lock();
        m_decoder->setFragment(m_offset, m_length);
        m_output2->seek(0); //reset time counter
        m_decoder->mutex()->unlock();
         // find next track
        if(track <= parser.count() - 1)
            m_nextUrl = parser.info(track + 1)->path();
        else
            m_nextUrl.clear();
        //is it track of another file?
        if(QUrl(m_nextUrl).path() != p)
            m_nextUrl.clear();
        //change track
        finish();
        //send metadata
        QMap<Qmmp::MetaData, QString> metaData = parser.info(track)->metaData();
        stateHandler()->dispatch(metaData);
    }
}

CUEStateHandler::CUEStateHandler(QObject *parent): StateHandler(parent){}

CUEStateHandler::~CUEStateHandler(){}

void CUEStateHandler::dispatch(qint64 elapsed,
                               int bitrate,
                               quint32 frequency,
                               int precision,
                               int channels)
{
    StateHandler::instance()->dispatch(elapsed, bitrate,
                                       frequency, precision, channels);

}

void CUEStateHandler::dispatch(const QMap<Qmmp::MetaData, QString> &metaData)
{
    //ignore media file metadata
    Q_UNUSED(metaData)
}

void CUEStateHandler::dispatch(const Qmmp::State &state)
{
    StateHandler::instance()->dispatch(state);
}
