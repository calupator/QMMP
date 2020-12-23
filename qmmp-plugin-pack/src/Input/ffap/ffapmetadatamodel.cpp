/***************************************************************************
 *   Copyright (C) 2011-2021 by Ilya Kotov                                 *
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

#include <QTextCodec>
#include <QRegularExpression>
#include <taglib/tag.h>
#include <taglib/id3v1tag.h>
#include <taglib/apetag.h>
#include <qmmp/metadatamanager.h>
#include "ffapmetadatamodel.h"

FFapMetaDataModel::FFapMetaDataModel(const QString &path, bool readOnly) : MetaDataModel(true)
{
    if(path.contains("://"))
    {
        QString p = path;
        p.remove("ape://");
        p.remove(QRegularExpression("#\\d+$"));
        m_path = p;
        m_stream = new TagLib::FileStream(QStringToFileName(p), true);
        m_file = new TagLib::APE::File(m_stream);
    }
    else
    {
        m_path = path;
        m_stream = new TagLib::FileStream(QStringToFileName(path), readOnly);
        m_file = new TagLib::APE::File(m_stream);
        m_tags << new FFapFileTagModel(m_file, TagLib::APE::File::ID3v1);
        m_tags << new FFapFileTagModel(m_file, TagLib::APE::File::APE);
        setReadOnly(readOnly);
    }
}

FFapMetaDataModel::~FFapMetaDataModel()
{
    while(!m_tags.isEmpty())
        delete m_tags.takeFirst();
    delete m_file;
    delete m_stream;
}

QList<MetaDataItem> FFapMetaDataModel::extraProperties() const
{
    QList<MetaDataItem> ep;
    TagLib::APE::Properties *ap = m_file->audioProperties();
    if(ap)
    {
        ep << MetaDataItem(tr("Samples"), ap->sampleFrames());
        ep << MetaDataItem(tr("Version"), ap->version());
    }
    return ep;
}

QList<TagModel* > FFapMetaDataModel::tags() const
{
    return m_tags;
}

QString FFapMetaDataModel::coverPath() const
{
    return MetaDataManager::instance()->findCoverFile(m_path);
}

FFapFileTagModel::FFapFileTagModel(TagLib::APE::File *file, TagLib::APE::File::TagTypes tagType) : TagModel(),
    m_file(file),
    m_tagType(tagType)
{
    if (m_tagType == TagLib::APE::File::ID3v1)
    {
        m_tag = m_file->ID3v1Tag();
        m_codec = QTextCodec::codecForName("ISO-8859-1");
    }
    else
    {
        m_tag = m_file->APETag();
        m_codec = QTextCodec::codecForName ("UTF-8");
    }
}

FFapFileTagModel::~FFapFileTagModel()
{}

QString FFapFileTagModel::name() const
{
    if (m_tagType == TagLib::APE::File::ID3v1)
        return "ID3v1";
    return "APE";
}

QList<Qmmp::MetaData> FFapFileTagModel::keys() const
{
    QList<Qmmp::MetaData> list = TagModel::keys();
    list.removeAll(Qmmp::COMPOSER);
    list.removeAll(Qmmp::DISCNUMBER);
    return list;
}

QString FFapFileTagModel::value(Qmmp::MetaData key) const
{
    if (m_tag)
    {
        bool utf = m_codec->name().contains("UTF");
        TagLib::String str;
        switch((int) key)
        {
        case Qmmp::TITLE:
            str = m_tag->title();
            break;
        case Qmmp::ARTIST:
            str = m_tag->artist();
            break;
        case Qmmp::ALBUM:
            str = m_tag->album();
            break;
        case Qmmp::COMMENT:
            str = m_tag->comment();
            break;
        case Qmmp::GENRE:
            str = m_tag->genre();
            break;
        case Qmmp::YEAR:
            return QString::number(m_tag->year());
        case Qmmp::TRACK:
            return QString::number(m_tag->track());
        }
        return m_codec->toUnicode(str.toCString(utf)).trimmed();
    }
    return QString();
}

void FFapFileTagModel::setValue(Qmmp::MetaData key, const QString &value)
{
    if(!m_tag)
        return;
    TagLib::String::Type type = TagLib::String::Latin1;

    if (m_tagType == TagLib::APE::File::ID3v1)
    {
        if(m_codec->name().contains("UTF")) //utf is unsupported
            return;
    }
    else if(m_tagType == TagLib::APE::File::APE)
        type = TagLib::String::UTF8;

    TagLib::String str = TagLib::String(m_codec->fromUnicode(value).constData(), type);

    switch((int) key)
    {
    case Qmmp::TITLE:
        m_tag->setTitle(str);
        break;
    case Qmmp::ARTIST:
        m_tag->setArtist(str);
        break;
    case Qmmp::ALBUM:
        m_tag->setAlbum(str);
        break;
    case Qmmp::COMMENT:
        m_tag->setComment(str);
        break;
    case Qmmp::GENRE:
        m_tag->setGenre(str);
        break;
    case Qmmp::YEAR:
        m_tag->setYear(value.toInt());
        break;
    case Qmmp::TRACK:
        m_tag->setTrack(value.toInt());
    }
}

bool FFapFileTagModel::exists() const
{
    return (m_tag != nullptr);
}

void FFapFileTagModel::create()
{
    if (m_tag)
        return;
    if (m_tagType == TagLib::APE::File::ID3v1)
        m_tag = m_file->ID3v1Tag(true);
    else
        m_tag = m_file->APETag(true);
}

void FFapFileTagModel::remove()
{
    m_tag = nullptr;
}

void FFapFileTagModel::save()
{
    if(!m_tag)
        m_file->strip(m_tagType);
    m_file->save();
}
