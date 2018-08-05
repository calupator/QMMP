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

#include <QRegExp>
#include <QMessageBox>
#include <taglib/apefile.h>
#include <taglib/apetag.h>
#include <taglib/tfilestream.h>
#include "ffapmetadatamodel.h"
#include "decoderffapfactory.h"
#include "decoder_ffap.h"
#include "decoder_ffapcue.h"
#include "cueparser.h"

// DecoderFFapFactory

bool DecoderFFapFactory::canDecode(QIODevice *input) const
{
    char buf[3];
    return (input->peek(buf, 3) == 3 && !memcmp(buf, "MAC", 3));
}

DecoderProperties DecoderFFapFactory::properties() const
{
    DecoderProperties properties;
    properties.name = tr("FFap Plugin");
    properties.filters << "*.ape";
    properties.description = tr("Monkey's Audio Files");
    //properties.contentType = ;
    properties.shortName = "ffap";
    properties.hasAbout = true;
    properties.hasSettings = false;
    properties.noInput = false;
    properties.protocols << "ape";
    properties.priority = 9;
    return properties;
}

Decoder *DecoderFFapFactory::create(const QString &url, QIODevice *i)
{
    if(url.contains("://"))
        return new DecoderFFapCUE(url);
    else
        return new DecoderFFap(url, i);
}

QList<TrackInfo *> DecoderFFapFactory::createPlayList(const QString &path, TrackInfo::Parts parts, QStringList *)
{
    //extract metadata of one cue track
    if(path.contains("://"))
    {
        QString filePath = path;
        filePath.remove("ape://");
        filePath.remove(QRegExp("#\\d+$"));
        int track = filePath.section("#", -1).toInt();
        QList<TrackInfo *> list = createPlayList(filePath, parts, 0);
        if (list.isEmpty() || track <= 0 || track > list.count())
        {
            qDeleteAll(list);
            list.clear();
            return list;
        }
        TrackInfo *info = list.takeAt(track - 1);
        qDeleteAll(list);
        return QList<TrackInfo *>() << info;
    }

    TrackInfo *info = new TrackInfo(path);

    if(parts == TrackInfo::NoParts)
        return QList<TrackInfo *>() << info;

    TagLib::FileStream stream(QStringToFileName(path), true);
    TagLib::APE::File file(&stream);
    TagLib::APE::Tag *tag = file.APETag();
    TagLib::APE::Properties *ap = file.audioProperties();

    if((parts & TrackInfo::MetaData) && tag && !tag->isEmpty())
    {
        if (tag->itemListMap().contains("CUESHEET"))
        {
            delete info;

            QByteArray data(tag->itemListMap()["CUESHEET"].toString().toCString(true));
            CUEParser parser(data, path);
            return parser.createPlayList();
        }

        info->setValue(Qmmp::ALBUM, TStringToQString(tag->album()));
        info->setValue(Qmmp::ARTIST, TStringToQString(tag->artist()));
        info->setValue(Qmmp::COMMENT, TStringToQString(tag->comment()));
        info->setValue(Qmmp::GENRE, TStringToQString(tag->genre()));
        info->setValue(Qmmp::TITLE, TStringToQString(tag->title()));
        info->setValue(Qmmp::YEAR, tag->year());
        info->setValue(Qmmp::TRACK, tag->track());
        //additional metadata
        TagLib::APE::Item fld;
        if(!(fld = tag->itemListMap()["ALBUM ARTIST"]).isEmpty())
            info->setValue(Qmmp::ALBUMARTIST, TStringToQString(fld.toString()));
        if(!(fld = tag->itemListMap()["COMPOSER"]).isEmpty())
            info->setValue(Qmmp::COMPOSER, TStringToQString(fld.toString()));
    }

    if((parts & TrackInfo::Properties) && ap)
    {
        info->setValue(Qmmp::BITRATE, ap->bitrate());
        info->setValue(Qmmp::SAMPLERATE, ap->sampleRate());
        info->setValue(Qmmp::CHANNELS, ap->channels());
        info->setValue(Qmmp::BITS_PER_SAMPLE, ap->bitsPerSample());
        info->setValue(Qmmp::FORMAT_NAME, "Monkey's Audio");
        info->setDuration(ap->lengthInMilliseconds());
    }

    if((parts & TrackInfo::ReplayGainInfo) && tag && !tag->isEmpty())
    {
        TagLib::APE::ItemListMap items = tag->itemListMap();
        if (items.contains("REPLAYGAIN_TRACK_GAIN"))
            info->setValue(Qmmp::REPLAYGAIN_TRACK_GAIN,TStringToQString(items["REPLAYGAIN_TRACK_GAIN"].values()[0]));
        if (items.contains("REPLAYGAIN_TRACK_PEAK"))
            info->setValue(Qmmp::REPLAYGAIN_TRACK_PEAK,TStringToQString(items["REPLAYGAIN_TRACK_PEAK"].values()[0]));
        if (items.contains("REPLAYGAIN_ALBUM_GAIN"))
            info->setValue(Qmmp::REPLAYGAIN_ALBUM_GAIN,TStringToQString(items["REPLAYGAIN_ALBUM_GAIN"].values()[0]));
        if (items.contains("REPLAYGAIN_ALBUM_PEAK"))
            info->setValue(Qmmp::REPLAYGAIN_ALBUM_PEAK,TStringToQString(items["REPLAYGAIN_ALBUM_PEAK"].values()[0]));
    }

    return QList<TrackInfo *>() << info;
}

MetaDataModel* DecoderFFapFactory::createMetaDataModel(const QString &path, bool readOnly, QObject *parent)
{
    return new FFapMetaDataModel(path, readOnly, parent);
}

void DecoderFFapFactory::showSettings(QWidget *)
{}

void DecoderFFapFactory::showAbout(QWidget *parent)
{
    QMessageBox::about (parent, tr("About FFap Audio Plugin"),
                        tr("Qmmp FFap Audio Plugin")+"\n"+
                        tr("This plugin provides Monkey's Audio (APE) support") +"\n"+
                        tr("Written by: Ilya Kotov <forkotov02@ya.ru>")  +"\n"+
                        tr("Based on code from deadbeef, FFmpeg and rockbox"));
}

QString DecoderFFapFactory::translation() const
{
    return QString(":/ffap_plugin_");
}
