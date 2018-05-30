/***************************************************************************
 *   Copyright (C) 2016-2018 by Ilya Kotov                                 *
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

#ifndef DECODERARCHIVE_H
#define DECODERARCHIVE_H

#include <QString>
#include <archive.h>
#include <qmmp/decoderfactory.h>
#include <qmmp/decoder.h>


class DecoderArchive : public Decoder
{
public:
    DecoderArchive(const QString &url);

    virtual ~DecoderArchive();

    bool initialize();
    qint64 totalTime() const;
    void seek(qint64 time);
    qint64 read(unsigned char *data, qint64 maxSize);
    int bitrate() const;

private:
    QString m_url;
    Decoder *m_decoder;
    QIODevice *m_input;

};

#endif // DECODERARCHIVE_H
