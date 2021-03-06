/***************************************************************************
 *   Copyright (C) 2017-2021 by Ilya Kotov                                 *
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

#ifndef FFVIDEODECODER_H
#define FFVIDEODECODER_H

#include <QString>

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/version.h>
#include <libavutil/mathematics.h>
#include <libavutil/dict.h>
}

class FFVideoDecoder
{
public:
    FFVideoDecoder();
    ~FFVideoDecoder();

    bool initialize(const QString &path);
    qint64 totalTime() const;
    void seek(qint64 time);

    AVFormatContext *formatContext() const;
    AVCodecContext *audioCodecContext() const;
    AVCodecContext *videoCodecContext() const;
    int audioIndex() const;
    int videoIndex() const;

private:
    AVFormatContext *m_formatContext = nullptr;
    AVCodecContext *m_audioCodecContext = nullptr;
    AVCodecContext *m_videoCodecContext = nullptr;
    int m_audioIndex = 0, m_videoIndex = 0;
    qint64 m_totalTime = 0;
};

#endif // FFVIDEODECODER_H
