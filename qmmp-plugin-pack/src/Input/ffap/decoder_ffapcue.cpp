/***************************************************************************
 *   Copyright (C) 2011-2020 by Ilya Kotov                                 *
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

#include <QObject>
#include <QFile>
#include <QRegularExpression>
#include <taglib/apefile.h>
#include <taglib/apetag.h>
#include <taglib/tfilestream.h>
#include <qmmp/cueparser.h>
#include "decoder_ffap.h"
#include "decoder_ffapcue.h"

DecoderFFapCUE::DecoderFFapCUE(const QString &url) : Decoder(),
    m_url(url)
{}

DecoderFFapCUE::~DecoderFFapCUE()
{
    if(m_decoder)
        delete m_decoder;
    m_decoder = nullptr;
    if(m_parser)
        delete m_parser;
    m_parser = nullptr;
    if(m_buf)
        delete [] m_buf;
    m_buf = nullptr;
    if(m_input)
        m_input->deleteLater();
    m_input = nullptr;
}

bool DecoderFFapCUE::initialize()
{
    QString filePath = m_url;
    if(!m_url.startsWith("ape://") || filePath.endsWith(".ape"))
    {
        qWarning("DecoderFFapCUE: invalid url.");
        return false;
    }
    filePath.remove("ape://");
    filePath.remove(QRegularExpression("#\\d+$"));
    m_track = m_url.section("#", -1).toInt();

    TagLib::FileStream stream(QStringToFileName(filePath), true);
    TagLib::APE::File file(&stream);

    TagLib::APE::Tag *tag = file.APETag();

    if(!tag || !tag->itemListMap().contains("CUESHEET"))
    {
        qWarning("DecoderFFapCUE: unable to find cuesheet comment.");
        return false;
    }

    m_parser = new CueParser(tag->itemListMap()["CUESHEET"].toString().toCString(true));
    m_parser->setDuration(file.audioProperties()->lengthInMilliseconds());
    m_parser->setUrl("ape", filePath);

    if(m_track > m_parser->count() || m_parser->isEmpty())
    {
        qWarning("DecoderFFapCUE: invalid cuesheet");
        return false;
    }
    m_input = new QFile(filePath);
    if(!m_input->open(QIODevice::ReadOnly))
    {
        qWarning("DecoderFFapCUE: %s", qPrintable(m_input->errorString()));
        return false;
    }
    QMap<Qmmp::MetaData, QString> metaData = m_parser->info(m_track)->metaData();
    addMetaData(metaData); //send metadata

    m_duration = m_parser->duration(m_track);
    m_offset = m_parser->offset(m_track);

    m_decoder = new DecoderFFap(filePath, m_input);
    if(!m_decoder->initialize())
    {
        qWarning("DecoderFFapCUE: invalid audio file");
        return false;
    }
    m_decoder->seek(m_offset);

    configure(m_decoder->audioParameters().sampleRate(),
              m_decoder->audioParameters().channels(),
              m_decoder->audioParameters().format());

    m_trackSize = audioParameters().sampleRate() *
            audioParameters().channels() *
            audioParameters().sampleSize() * m_duration / 1000;
    m_written = 0;

    m_frameSize = audioParameters().sampleSize() * audioParameters().channels();

    setReplayGainInfo(m_parser->info(m_track)->replayGainInfo()); //send ReplayGaing info
    addMetaData(m_parser->info(m_track)->metaData()); //send metadata
    return true;
}

qint64 DecoderFFapCUE::totalTime() const
{
    return m_decoder ? m_duration : 0;
}

void DecoderFFapCUE::seek(qint64 pos)
{
    m_decoder->seek(m_offset + pos);
    m_written = audioParameters().sampleRate() *
            audioParameters().channels() *
            audioParameters().sampleSize() * pos/1000;
}

qint64 DecoderFFapCUE::read(unsigned char *data, qint64 size)
{
    if(m_trackSize - m_written < m_frameSize) //end of cue track
        return 0;

    qint64 len = 0;

    if(m_buf) //read remaining data first
    {
        len = qMin(m_buf_size, size);
        memmove(data, m_buf, len);
        if(size >= m_buf_size)
        {
            delete[] m_buf;
            m_buf = nullptr;
            m_buf_size = 0;
        }
        else
            memmove(m_buf, m_buf + len, size - len);
    }
    else
        len = m_decoder->read(data, size);

    if(len <= 0) //end of file
        return 0;

    if(len + m_written <= m_trackSize)
    {
        m_written += len;
        return len;
    }

    qint64 len2 = qMax(qint64(0), m_trackSize - m_written);
    len2 = (len2 / m_frameSize) * m_frameSize; //whole of samples of each channel
    m_written += len2;
    //save data of the next track
    if(m_buf)
        delete[] m_buf;
    m_buf_size = len - len2;
    m_buf = new char[m_buf_size];
    memmove(m_buf, data + len2, m_buf_size);
    return len2;
}

int DecoderFFapCUE::bitrate() const
{
    return m_decoder->bitrate();
}

const QString DecoderFFapCUE::nextURL() const
{
    if(m_track +1 <= m_parser->count())
        return m_parser->url(m_track + 1);
    else
        return QString();
}

void DecoderFFapCUE::next()
{
    if(m_track +1 <= m_parser->count())
    {
        m_track++;
        m_duration = m_parser->duration(m_track);
        m_offset = m_parser->offset(m_track);
        m_trackSize = audioParameters().sampleRate() *
                audioParameters().channels() *
                audioParameters().sampleSize() * m_duration/1000;
        addMetaData(m_parser->info(m_track)->metaData());
        setReplayGainInfo(m_parser->info(m_track)->replayGainInfo());
        m_written = 0;
    }
}
