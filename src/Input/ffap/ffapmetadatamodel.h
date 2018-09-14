/***************************************************************************
 *   Copyright (C) 2011-2018 by Ilya Kotov                                 *
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

#ifndef FFAPMETADATAMODEL_H
#define FFAPMETADATAMODEL_H

#include <qmmp/metadatamodel.h>
#include <taglib/tag.h>
#include <taglib/apefile.h>
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 8))
#include <taglib/tfilestream.h>
#endif

class QTextCodec;

/**
    @author Ilya Kotov <forkotov02@ya.ru>
*/
class FFapMetaDataModel : public MetaDataModel
{
    Q_DECLARE_TR_FUNCTIONS(FFapMetaDataModel)
public:
    FFapMetaDataModel(const QString &path, bool readOnly);
    ~FFapMetaDataModel();
    QList<MetaDataItem> extraProperties() const;
    QList<TagModel* > tags() const;
    QString coverPath() const;

private:
    QString m_path;
    QList<TagModel* > m_tags;
    TagLib::APE::File *m_file;
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 8))
    TagLib::FileStream *m_stream;
#endif
};

class FFapFileTagModel : public TagModel
{
public:
    FFapFileTagModel(TagLib::APE::File *file, TagLib::APE::File::TagTypes tagType);
    ~FFapFileTagModel();
    QString name() const;
    QList<Qmmp::MetaData> keys() const;
    QString value(Qmmp::MetaData key) const;
    void setValue(Qmmp::MetaData key, const QString &value);
    bool exists() const;
    void create();
    void remove();
    void save();

private:
    QTextCodec *m_codec;
    TagLib::APE::File *m_file;
    TagLib::Tag *m_tag;
    TagLib::APE::File::TagTypes m_tagType;
};

#endif // FFapMETADATAMODEL_H
