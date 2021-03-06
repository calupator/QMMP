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

#ifndef YTBINPUTSOURCE_H
#define YTBINPUTSOURCE_H

#include <QNetworkAccessManager>
#include <QProcess>
#include <qmmp/inputsource.h>

class QNetworkReply;
class BufferDevice;

/**
    @author Ilya Kotov <forkotov02@ya.ru>
*/
class YtbInputSource : public InputSource
{
Q_OBJECT
public:
    explicit YtbInputSource(const QString &path, QObject *parent = nullptr);
    ~YtbInputSource();

    QIODevice *ioDevice() const override;
    bool initialize() override;
    bool isReady() const override;
    bool isWaiting() const override;
    QString contentType() const override;
    void stop() override;


private slots:
    void onProcessErrorOccurred(QProcess::ProcessError);
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onFinished(QNetworkReply *reply);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onSeekRequest();

private:
    QString m_url;
    bool m_ready = false;
    QProcess *m_process;
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_getStreamReply = nullptr;
    BufferDevice *m_buffer;
    qint64 m_fileSize = -1;
    qint64 m_offset = 0;
    QNetworkRequest m_request;

};

#endif // YTBINPUTSOURCE_H
