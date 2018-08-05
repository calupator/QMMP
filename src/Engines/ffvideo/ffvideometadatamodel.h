/***************************************************************************
 *   Copyright (C) 2017-2018 by Ilya Kotov                                  *
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

#ifndef FFVIDEOMETADATAMODEL_H
#define FFVIDEOMETADATAMODEL_H

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/dict.h>
}

#include <qmmp/metadatamodel.h>

class FFVideoMetaDataModel : public MetaDataModel
{
Q_OBJECT
public:
    FFVideoMetaDataModel(const QString &path, QObject *parent);
    ~FFVideoMetaDataModel();
    QList<MetaDataItem> extraProperties() const;
    QPixmap cover() const;

private:
    AVFormatContext *m_in;
};

#endif // FFVIDEOMETADATAMODEL_H
