/***************************************************************************
 *   Copyright (C) 2006 by Ilya Kotov                                      *
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
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/mpcfile.h>

#include "detailsdialog.h"

DetailsDialog::DetailsDialog(QWidget *parent, const QString &path)
        : QDialog(parent)
{
    ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    m_path = path;
    setWindowTitle (path.section('/',-1));
    path.section('/',-1);
    ui.pathLineEdit->setText(m_path);
    loadMPCInfo();
    loadTag();

}


DetailsDialog::~DetailsDialog()
{}

void DetailsDialog::loadMPCInfo()
{
    TagLib::MPC::File f (m_path.toLocal8Bit());
    QString text;
    text = QString("%1").arg(f.audioProperties()->length()/60);
    text +=":"+QString("%1").arg(f.audioProperties()->length()%60,2,10,QChar('0'));
    ui.lengthLabel->setText(text);
    text = QString("%1").arg(f.audioProperties()->sampleRate());
    ui.sampleRateLabel->setText(text+" "+tr("Hz"));
    text = QString("%1").arg(f.audioProperties()->channels());
    ui.channelsLabel->setText(text);
    text = QString("%1").arg(f.audioProperties()->bitrate());
    ui.bitrateLabel->setText(text+" "+tr("kbps")); 
    text = QString("%1").arg(f.audioProperties()->mpcVersion());
    ui.versionLabel->setText(text);
    text = QString("%1 "+tr("KB")).arg(f.length()/1024);
    ui.fileSizeLabel->setText(text);
}

void DetailsDialog::loadTag()
{
    TagLib::FileRef f (m_path.toLocal8Bit());

    if (f.tag())
    {   //TODO: load codec name from config

        TagLib::String title = f.tag()->title();
        TagLib::String artist = f.tag()->artist();
        TagLib::String album = f.tag()->album();
        TagLib::String comment = f.tag()->comment();
        TagLib::String genre = f.tag()->genre();
        QString string = QString::fromUtf8(title.toCString(TRUE)).trimmed();
        ui.titleLineEdit->setText(string);
        string = QString::fromUtf8(artist.toCString(TRUE)).trimmed();
        ui.artistLineEdit->setText(string);
        string = QString::fromUtf8(album.toCString(TRUE)).trimmed();
        ui.albumLineEdit->setText(string);
        string = QString::fromUtf8(comment.toCString(TRUE)).trimmed();
        ui.commentLineEdit->setText(string);
        string = QString("%1").arg(f.tag()->year());
        ui.yearLineEdit->setText(string);
        string = QString("%1").arg(f.tag()->track());
        ui.trackLineEdit->setText(string);
        string = QString::fromUtf8(genre.toCString(TRUE)).trimmed();
        ui.genreLineEdit->setText(string);
    }
}

