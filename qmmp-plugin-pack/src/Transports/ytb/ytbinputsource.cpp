/***************************************************************************
 *   Copyright (C) 2009-2013 by Ilya Kotov                                 *
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

//#include "ytbstreamreader.h"
#include <QtDebug>
#include <QNetworkReply>
#include <QUrlQuery>
#include "ytbinputsource.h"

YtbInputSource::YtbInputSource(const QString &url, QObject *parent) : InputSource(url,parent)
{
    m_url = url;
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(onFinished(QNetworkReply*)));
    qDebug() << Q_FUNC_INFO << url;
     /*m_reader = new HttpStreamReader(url, this);
    connect(m_reader, SIGNAL(ready()),SIGNAL(ready()));
    connect(m_reader, SIGNAL(error()),SIGNAL(error()));*/
}

QIODevice *YtbInputSource::ioDevice()
{
    return m_getStreamReply;
}

bool YtbInputSource::initialize()
{
    QString id = m_url.section("://", -1);
    QUrl getVideoUrl(QString("https://www.youtube.com/get_video_info?video_id=%1").arg(id));
    QNetworkRequest request(getVideoUrl);
    request.setRawHeader("Host", getVideoUrl.host().toLatin1());
    request.setRawHeader("Accept", "*/*");
    m_getVideoReply = m_manager->get(request);
    return true;
}

bool YtbInputSource::isReady()
{
    return m_ready;
}

bool YtbInputSource::isWaiting()
{
    return false;
    //return (!m_reader->bytesAvailable() && m_reader->isOpen());
}

QString YtbInputSource::contentType() const
{
    return QString();
    //return m_reader->contentType();
}

void YtbInputSource::onFinished(QNetworkReply *reply)
{
    if(m_getVideoReply == reply)
    {
        m_getVideoReply = nullptr;

        QString data = QString::fromLatin1(reply->readAll());
        QUrlQuery query(data);
        QString fmts = query.queryItemValue("adaptive_fmts");
        query.setQuery(fmts);

//        for(const QString &bitrate : query.allQueryItemValues("audio_sample_rate", QUrl::FullyDecoded))
//            qDebug() << bitrate;

//        for(const QString &bitrate : query.allQueryItemValues("audio_channels", QUrl::FullyDecoded))
//            qDebug() << bitrate;

        for(const QString &encodedUrl : query.allQueryItemValues("url", QUrl::FullyDecoded))
        {
            QByteArray tmp = QByteArray::fromPercentEncoding(QByteArray::fromPercentEncoding(encodedUrl.toLatin1()));
            QString streamUrl = QString::fromLatin1(tmp);
            QUrlQuery streamQuery(streamUrl);
            QString mime = streamQuery.queryItemValue("mime", QUrl::FullyDecoded);

            if(mime == "audio/mp4")
            {
                QUrl ur(streamUrl);
                QNetworkRequest request(ur);
                request.setRawHeader("Host", ur.host().toLatin1());
                request.setRawHeader("Accept", "*/*");
                m_getStreamReply = m_manager->get(request);
                m_getStreamReply->setReadBufferSize(0);
                connect(m_getStreamReply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(onDownloadProgress(qint64,qint64)));
                break;
            }
        }

        reply->deleteLater();
    }

    if(m_getStreamReply == reply)
    {
        //m_getStreamReply = nullptr;
        qDebug() << reply->errorString();
    }


}

void YtbInputSource::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug() << bytesReceived << bytesTotal;
    if(!m_ready && bytesReceived > 500000)
    {
        m_ready = true;
        emit ready();
    }
}
