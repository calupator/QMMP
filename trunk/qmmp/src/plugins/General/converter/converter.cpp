/***************************************************************************
 *   Copyright (C) 2011 by Ilya Kotov                                      *
 *   forkotov02@hotmail.ru                                                 *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <QStringList>
#include <qmmp/inputsourcefactory.h>
#include <qmmp/decoderfactory.h>
#include <QtEndian>
#include <math.h>
#include "converter.h"

Converter::Converter(QObject *parent) : QThread(parent)
{}

void Converter::add(const QStringList &urls)
{
    foreach(QString url, urls)
        add(url);

}

void Converter::add(const QString &url)
{
    InputSource *source = InputSource::create(url, this);
    if(!source->initialize())
    {
        delete source;
        qWarning("Converter: Invalid url");
        return;
    }

    if(source->ioDevice())
    {
        source->ioDevice()->open(QIODevice::ReadOnly);
    }

    DecoderFactory *factory = 0;

    if(!source->url().contains("://"))
        factory = Decoder::findByPath(source->url());
    if(!factory)
        factory = Decoder::findByMime(source->contentType());
    if(!factory && source->ioDevice() && source->url().contains("://")) //ignore content of local files
        factory = Decoder::findByContent(source->ioDevice());
    if(!factory && source->url().contains("://"))
        factory = Decoder::findByProtocol(source->url().section("://",0,0));
    if(!factory)
    {
        qWarning("Converter: unsupported file format");
        return;
    }
    qDebug("Converter: selected decoder: %s",qPrintable(factory->properties().shortName));
    if(factory->properties().noInput && source->ioDevice())
        source->ioDevice()->close();
    Decoder *decoder = factory->create(source->url(), source->ioDevice());
    if(!decoder->initialize())
    {
        qWarning("Converter: invalid file format");
        delete decoder;
        return;
    }
    m_decoders.enqueue(decoder);
    m_inputs.insert(decoder, source);
    if(!decoder->totalTime())
        source->setOffset(-1);
    source->setParent(this);
}

void Converter::run()
{
    while(!m_decoders.isEmpty())
    {
        Decoder *decoder = m_decoders.dequeue();
        AudioParameters ap = decoder->audioParameters();

        char wave_header[] = { 0x52, 0x49, 0x46, 0x46, //"RIFF"
                              0x00, 0x00, 0x00, 0x00, //(file size) - 8
                              0x57, 0x41, 0x56, 0x45, //WAVE
                              0x66, 0x6d, 0x74, 0x20, //"fmt "
                              0x10, 0x00, 0x00, 0x00, //16 + extra format bytes
                              0x01, 0x00, 0x02, 0x00, //PCM/uncompressed, channels
                              0x00, 0x00, 0x00, 0x00, //sample rate
                              0x00, 0x00, 0x00, 0x00, //average bytes per second
                              0x04, 0x00, 0x10, 0x00, //block align, significant bits per sample
                              0x64, 0x61, 0x74, 0x61, //"data"
                              0x00, 0x00, 0x00, 0x00 }; //chunk size*/

        quint32 sample_rate = qToLittleEndian(ap.sampleRate());
        quint16 channels = qToLittleEndian((quint16)ap.channels());
        quint16 block_align = qToLittleEndian((quint16)ap.sampleSize() * ap.channels());
        quint16 bps = qToLittleEndian((quint16)ap.sampleSize() * 8);
        quint32 size = decoder->totalTime() * ap.sampleRate() * ap.channels() * ap.sampleSize() / 1000;
        size = qToLittleEndian(size);

        memcpy(&wave_header[22], &channels, 2);
        memcpy(&wave_header[24], &sample_rate, 4);
        memcpy(&wave_header[32], &block_align, 2);
        memcpy(&wave_header[34], &bps, 2);
        memcpy(&wave_header[40], &size, 4);

        //FILE *enc_pipe = fopen("/mnt/win_e/out.wav", "w");
        FILE *enc_pipe = popen("oggenc -q 1 -o \"/mnt/win_e/out 55.ogg\" -", "w");
        if(!enc_pipe)
        {
            qWarning("Converter: unable to open pipe");
            delete decoder;
            continue;

        }
        size_t to_write = sizeof(wave_header);
        if(to_write != fwrite(&wave_header, 1, to_write, enc_pipe))
        {
            delete decoder;
            pclose(enc_pipe);
            continue;
        }

        convert(decoder, enc_pipe);
        pclose(enc_pipe);
        //fclose(enc_pipe);
        delete decoder;
    }
}

bool Converter::convert(Decoder *decoder, FILE *file)
{
    const int buf_size = 8192;
    unsigned char output_buf[buf_size];
    qint64 output_at = 0;
    qint64 total = 0;
    quint64 len = 0;
    AudioParameters ap = decoder->audioParameters();
    qint64 size = decoder->totalTime() * ap.sampleRate() * ap.channels() * ap.sampleSize() / 1000;
    forever
    {
        // decode
        len = decoder->read((char *)(output_buf + output_at), buf_size - output_at);

        if (len > 0)
        {
            output_at += len;
            total += len;
            while(output_at > 0)
            {
                len = fwrite(output_buf, 1, output_at, file);
                if(len == 0)
                {
                    qWarning("Converter: error");
                    return false;
                }
                output_at -= len;
                memmove(output_buf, output_buf + len, output_at);
            }
            emit progress(floor(100*total/size));
        }
        else if (len <= 0)
        {
            qDebug("Converter: total written: %lld bytes", total);
            qDebug("finished!");
            return true;
        }
    }
    return false;
}

