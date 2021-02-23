/***************************************************************************
 *   Copyright (C) 2009-2019 by Ilya Kotov                                 *
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

#include <QBuffer>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/vorbisfile.h>
#include <taglib/xiphcomment.h>
#include <taglib/tmap.h>
#include "vorbismetadatamodel.h"

#define QStringToTString_qt4(s) TagLib::String(s.toUtf8().constData(), TagLib::String::UTF8)

VorbisMetaDataModel::VorbisMetaDataModel(const QString &path, bool readOnly)
#ifdef HAS_PICTURE_LIST
    : MetaDataModel(readOnly, MetaDataModel::IS_COVER_EDITABLE)
#else
    : MetaDataModel(readOnly)
#endif
{
    m_path = path;
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 8))
    m_stream = new TagLib::FileStream(QStringToFileName(path), readOnly);
    m_file = new TagLib::Ogg::Vorbis::File(m_stream);
#else
    m_file = new TagLib::Ogg::Vorbis::File(QStringToFileName(path));
#endif
    m_tag = m_file->tag();
    m_tags << new VorbisCommentModel(this);
}

VorbisMetaDataModel::~VorbisMetaDataModel()
{
    while(!m_tags.isEmpty())
        delete m_tags.takeFirst();

    delete m_file;
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 8))
    delete m_stream;
#endif
}

QList<TagModel* > VorbisMetaDataModel::tags() const
{
    return m_tags;
}

QPixmap VorbisMetaDataModel::cover() const
{
    if(!m_tag || m_tag->isEmpty())
        return QPixmap();
    TagLib::StringList list = m_tag->fieldListMap()["METADATA_BLOCK_PICTURE"];
    if(list.isEmpty())
        return QPixmap();
    for(uint i = 0; i < list.size(); ++i)
    {
        TagLib::String value = list[i];
        QByteArray block = QByteArray::fromBase64(TStringToQString(value).toLatin1());
        if(block.size() < 32)
            continue;
        qint64 pos = 0;
        if(readPictureBlockField(block, pos) != 3) //picture type, use front cover only
            continue;
        pos += 4;
        int mimeLength = readPictureBlockField(block, pos); //mime type length
        pos += 4;
        pos += mimeLength; //skip mime type
        int descLength = readPictureBlockField(block, pos); //description length
        pos += 4;
        pos += descLength; //skip description
        pos += 4; //width
        pos += 4; //height
        pos += 4; //color depth
        pos += 4; //the number of colors used
        int length = readPictureBlockField(block, pos); //picture size
        pos += 4;
        QPixmap cover;
        cover.loadFromData(block.mid(pos, length)); //read binary picture data
        return cover;
    }
    return QPixmap();
}

#ifdef HAS_PICTURE_LIST
void VorbisMetaDataModel::setCover(const QPixmap &pix)
{
    removeCover();
    if(m_tag)
    {
        TagLib::FLAC::Picture *picture = new TagLib::FLAC::Picture();
        picture->setType(TagLib::FLAC::Picture::FrontCover);

        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        pix.save(&buffer, "JPEG");
        picture->setMimeType("image/jpeg");
        picture->setData(TagLib::ByteVector(data.constData(), data.size()));
        m_tag->addPicture(picture);
        m_file->save();
    }
}

void VorbisMetaDataModel::removeCover()
{
    if(m_tag && !m_tag->isEmpty())
    {
        bool save = false;
        TagLib::List<TagLib::FLAC::Picture *> list = m_tag->pictureList();
        for(uint i = 0; i < list.size(); ++i)
        {
            if(list[i]->type() == TagLib::FLAC::Picture::FrontCover)
            {
                m_tag->removePicture(list[i], false);
                save = true;
            }
        }
        if(save)
        {
            m_file->save();
        }
    }
}
#endif

ulong VorbisMetaDataModel::readPictureBlockField(QByteArray data, int offset) const
{
    return (((uchar)data.data()[offset] & 0xff) << 24) |
           (((uchar)data.data()[offset+1] & 0xff) << 16) |
           (((uchar)data.data()[offset+2] & 0xff) << 16) |
           ((uchar)data.data()[offset+3] & 0xff);

}

VorbisCommentModel::VorbisCommentModel(VorbisMetaDataModel *model) : TagModel(TagModel::Save)
{
    m_model = model;
}

VorbisCommentModel::~VorbisCommentModel()
{}

QString VorbisCommentModel::name() const
{
    return "Vorbis Comment";
}

QString VorbisCommentModel::value(Qmmp::MetaData key) const
{
    if(!m_model->m_tag || m_model->m_tag->isEmpty())
        return QString();

    TagLib::Ogg::XiphComment *tag  = m_model->m_tag;
    switch((int) key)
    {
    case Qmmp::TITLE:
        return TStringToQString(tag->title());
    case Qmmp::ARTIST:
        return TStringToQString(tag->artist());
    case Qmmp::ALBUMARTIST:
        if(tag->fieldListMap()["ALBUMARTIST"].isEmpty())
            return QString();
        else
            return TStringToQString(tag->fieldListMap()["ALBUMARTIST"].toString());
    case Qmmp::ALBUM:
        return TStringToQString(tag->album());
    case Qmmp::COMMENT:
        return TStringToQString(tag->comment());
    case Qmmp::GENRE:
        return TStringToQString(tag->genre());
    case Qmmp::COMPOSER:
        if(tag->fieldListMap()["COMPOSER"].isEmpty())
            return QString();
        else
            return TStringToQString(tag->fieldListMap()["COMPOSER"].toString());
    case Qmmp::YEAR:
        return QString::number(tag->year());
    case Qmmp::TRACK:
        return QString::number(tag->track());
    case  Qmmp::DISCNUMBER:
        if(tag->fieldListMap()["DISCNUMBER"].isEmpty())
            return QString();
        else
            return TStringToQString(tag->fieldListMap()["DISCNUMBER"].toString());
    }
    return QString();
}

void VorbisCommentModel::setValue(Qmmp::MetaData key, const QString &value)
{
    if(!m_model->m_tag || m_model->m_tag->isEmpty())
        return;

    TagLib::Ogg::XiphComment *tag = m_model->m_tag;

    TagLib::String str = QStringToTString_qt4(value);

    switch((int) key)
    {
    case Qmmp::TITLE:
        tag->setTitle(str);
        return;
    case Qmmp::ARTIST:
        tag->setArtist(str);
        return;
    case Qmmp::ALBUM:
        tag->setAlbum(str);
        return;
    case Qmmp::ALBUMARTIST:
        tag->addField("ALBUMARTIST", str, true);
        return;
    case Qmmp::COMMENT:
        tag->setComment(str);
        return;
    case Qmmp::GENRE:
        tag->setGenre(str);
        return;
    case Qmmp::COMPOSER:
        tag->addField("COMPOSER", str, true);
        return;
    case Qmmp::TRACK:
        tag->setTrack(value.toInt());
        return;
    case Qmmp::YEAR:
        tag->setYear(value.toInt());
        return;
    case Qmmp::DISCNUMBER:
        value == "0" ?
#if ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION <= 11))
        tag->removeField("DISCNUMBER"):
#else
        tag->removeFields("DISCNUMBER"):
#endif
        tag->addField("DISCNUMBER", str, true);
    }
}

void VorbisCommentModel::save()
{
    if(m_model->m_tag)
        m_model->m_file->save();

#if ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION <= 10))
    //taglib bug workarround
    delete m_model->m_file;
    m_model->m_file = new TagLib::Ogg::Vorbis::File(QStringToFileName(m_model->m_path));
    m_model->m_tag = m_model->m_file->tag();
#endif
}
