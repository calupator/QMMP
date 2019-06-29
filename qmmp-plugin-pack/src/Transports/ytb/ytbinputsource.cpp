/***************************************************************************
 *   Copyright (C) 2019 by Ilya Kotov                                      *
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

#include <QtDebug>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <qmmp/statehandler.h>
#include "bufferdevice.h"
#include "ytbinputsource.h"

#define PREBUFFER_SIZE 128000

YtbInputSource::YtbInputSource(const QString &url, QObject *parent) : InputSource(url, parent)
{
    m_url = url;
    m_buffer = new BufferDevice(this);
    m_process = new QProcess(this);
    m_manager = new QNetworkAccessManager(this);
    m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), SLOT(onProcessErrorOccurred(QProcess::ProcessError)));
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(onProcessFinished(int,QProcess::ExitStatus)));
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), SLOT(onFinished(QNetworkReply*)));
}

YtbInputSource::~YtbInputSource()
{
    if(m_getStreamReply)
    {
        if(m_getStreamReply->isFinished())
            m_getStreamReply->abort();
        m_getStreamReply->deleteLater();
        m_getStreamReply = nullptr;
    }
}

QIODevice *YtbInputSource::ioDevice()
{
    return m_buffer;
}

bool YtbInputSource::initialize()
{
    QString id = m_url.section("://", -1);
    QString cmd = QString("youtube-dl --print-json -s https://www.youtube.com/watch?v=%1").arg(id);

    m_ready = false;
    m_buffer->open(QIODevice::ReadOnly);
    m_process->start(cmd);
    qDebug("YtbInputSource: starting youtube-dl...");
    return true;
}

bool YtbInputSource::isReady()
{
    return m_ready;
}

bool YtbInputSource::isWaiting()
{
    return false;
}

QString YtbInputSource::contentType() const
{
    return QString();
    //return m_reader->contentType();
}

void YtbInputSource::onProcessErrorOccurred(QProcess::ProcessError)
{
    qWarning("YtbInputSource: unable to start process 'youtube-dl', error: %s", qPrintable(m_process->errorString()));
    emit error();
}

void YtbInputSource::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    if(exitCode != EXIT_SUCCESS || status != QProcess::NormalExit)
    {
        qWarning("YtbInputSource: youtube-dl finished with error:\n%s", m_process->readAllStandardError().constData());
        emit error();
        return;
    }

    QJsonDocument json = QJsonDocument::fromJson(m_process->readAllStandardOutput());
    if(json.isEmpty())
    {
        qWarning("YtbInputSource: unable to parse youtube-dl output");
        emit error();
        return;
    }

    //qDebug("%s", json.toJson(QJsonDocument::Indented).constData());

    QMap<Qmmp::MetaData, QString> metaData = {
        { Qmmp::TITLE, json["fulltitle"].toString() }
    };
    addMetaData(metaData);

    QHash<QString, QString> streamInfo = {
        { tr("Uploader"), json["uploader"].toString() },
        { tr("Upload date"), json["upload_date"].toString() },
        { tr("Duration"), QString("%1:%2").arg(json["duration"].toInt() / 60)
          .arg(json["duration"].toInt() % 60, 2, 10, QChar('0')) },
    };
    addStreamInfo(streamInfo);

    int bitrate = json["abr"].toInt();
    QString acodec = json["acodec"].toString();

    QString url;
    for(const QJsonValue &value : json["formats"].toArray())
    {
        qDebug() << value["acodec"].toString() << value["vcodec"].toString() << value["abr"].toInt();

        if(value["abr"].toInt() == bitrate &&
                value["acodec"].toString() == acodec &&
                value["vcodec"].toString() == "none")
        {
            url = value["url"].toString();
            break;
        }
    }

    if(url.isEmpty())
    {
        qWarning("YtbInputSource: unable to find stream");
        emit error();
        return;
    }

    qDebug("YtbInputSource: downloading stream...");

    QUrl streamUrl(url);
    QNetworkRequest request(streamUrl);
    request.setRawHeader("Host", streamUrl.host().toLatin1());
    request.setRawHeader("Accept", "*/*");
    m_getStreamReply = m_manager->get(request);
    m_getStreamReply->setReadBufferSize(0);
    connect(m_getStreamReply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(onDownloadProgress(qint64,qint64)));
}

void YtbInputSource::onFinished(QNetworkReply *reply)
{
    if(reply == m_getStreamReply)
    {
        if(reply->error() != QNetworkReply::NoError)
        {
            qWarning("YtbInputSource: downloading finished with error: %s", qPrintable(reply->errorString()));
            if(!m_ready)
            {
                m_getStreamReply = nullptr;
                reply->deleteLater();
                emit error();
            }
        }
        else
        {
            m_buffer->addData(m_getStreamReply->readAll());
            qDebug("YtbInputSource: downloading finished");
        }
    }
    else
    {
        reply->deleteLater();
    }
}

void YtbInputSource::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    Q_UNUSED(bytesTotal);

    if(!m_ready && bytesReceived > PREBUFFER_SIZE)
    {
        qDebug("YtbInputSource: ready");
        m_ready = true;
        m_buffer->open(QIODevice::ReadOnly);
        emit ready();
    }
    else if(!m_ready)
    {
        StateHandler::instance()->dispatchBuffer(100 * bytesReceived / PREBUFFER_SIZE);
    }

    m_buffer->addData(m_getStreamReply->readAll());
}
