/***************************************************************************
 *   Copyright (C) 2009-2018 by Ilya Kotov                                 *
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
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPushButton>
#include <QStyle>
#include <qmmp/metadatamanager.h>
#include <qmmp/metadatamodel.h>
#include <qmmp/tagmodel.h>
#include <qmmp/soundcore.h>
#include "ui_detailsdialog.h"
#include "metadataformatter.h"
#include "playlisttrack.h"
#include "tageditor_p.h"
#include "coverviewer_p.h"
#include "detailsdialog.h"

DetailsDialog::DetailsDialog(QList<PlayListTrack *> tracks, QWidget *parent)
        : QDialog(parent)
{
    m_ui = new Ui::DetailsDialog;
    m_ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose, true);
    m_ui->directoryButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon));
    m_ui->nextButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowRight));
    m_ui->prevButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowLeft));
    m_metaDataModel = 0;
    m_page = 0;
    m_tracks = tracks;
    updatePage();
    on_tabWidget_currentChanged(0);
}

DetailsDialog::~DetailsDialog()
{
    if(m_metaDataModel)
    {
        delete m_metaDataModel;
        m_metaDataModel = 0;
    }
    delete m_ui;
}

void DetailsDialog:: on_directoryButton_clicked()
{
    QString dir_path;
    if(!m_info.path().contains("://")) //local file
        dir_path = QFileInfo(m_info.path()).absolutePath();
    else if (m_info.path().contains(":///")) //pseudo-protocol
    {
        dir_path = QUrl(m_info.path()).path();
        dir_path.replace(QString(QUrl::toPercentEncoding("#")), "#");
        dir_path.replace(QString(QUrl::toPercentEncoding("?")), "?");
        dir_path.replace(QString(QUrl::toPercentEncoding("%")), "%");
        dir_path = QFileInfo(dir_path).absolutePath();
    }
    else
        return;

    QDesktopServices::openUrl(QUrl::fromLocalFile(dir_path));
}

void DetailsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if(m_ui->buttonBox->standardButton(button) == QDialogButtonBox::Save)
    {
        TagEditor *tab = qobject_cast<TagEditor *> (m_ui->tabWidget->currentWidget());
        if(tab)
            tab->save();
    }
    else
        reject();

    //close all files before closing dialog
    if(m_metaDataModel)
    {
        delete m_metaDataModel;
        m_metaDataModel = 0;
    }
}

void DetailsDialog::on_tabWidget_currentChanged(int index)
{
    TagEditor *tab = qobject_cast<TagEditor *> (m_ui->tabWidget->widget(index));
    m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(tab && m_metaDataModel && !m_metaDataModel->isReadOnly());
}

void DetailsDialog::on_prevButton_clicked()
{
    if(m_page == 0)
        m_page = m_tracks.count() - 1;
    else
        m_page--;
    updatePage();
}

void DetailsDialog::on_nextButton_clicked()
{
    if(m_page >= m_tracks.count() - 1)
        m_page = 0;
    else
        m_page++;
    updatePage();
}

void DetailsDialog::closeEvent(QCloseEvent *)
{
    //close all files before closing dialog
    if(m_metaDataModel)
    {
        delete m_metaDataModel;
        m_metaDataModel = 0;
    }
}

void DetailsDialog::updatePage()
{
    if(m_metaDataModel)
    {
        delete m_metaDataModel;
        m_metaDataModel = 0;
    }

    while (m_ui->tabWidget->count() > 1)
    {
        int index = m_ui->tabWidget->count() - 1;
        QWidget *w = m_ui->tabWidget->widget(index);
        m_ui->tabWidget->removeTab(index);
        w->deleteLater();
    }

    m_ui->pageLabel->setText(tr("%1/%2").arg(m_page + 1).arg(m_tracks.count()));
    m_info = *m_tracks.at(m_page);

    setWindowTitle(m_info.path().section('/',-1));
    m_ui->pathEdit->setText(m_info.path());

    //load metadata and create metadata model
    QList<TrackInfo *> infoList = MetaDataManager::instance()->createPlayList(m_info.path());
    if(!infoList.isEmpty())
    {
        if(infoList.first()->parts() && TrackInfo::MetaData)
            m_info.setValues(infoList.first()->metaData());
        if(infoList.first()->parts() && TrackInfo::Properties)
        {
            m_info.updateValues(infoList.first()->properties());
            m_info.setDuration(infoList.first()->duration());
        }
    }
    qDeleteAll(infoList);
    infoList.clear();

    QPixmap cover = MetaDataManager::instance()->getCover(m_info.path());
    if(!cover.isNull())
    {
        CoverViewer *coverViewer = new CoverViewer(this);
        coverViewer->setPixmap(cover);
        m_ui->tabWidget->addTab(coverViewer, tr("Cover"));
    }

    if(m_info.path().contains("://")) //URL
    {
        m_metaDataModel = MetaDataManager::instance()->createMetaDataModel(m_info.path(), false);
    }
    else if(QFile::exists(m_info.path())) //local file
    {
        bool writable = QFileInfo(m_info.path()).isWritable();
        m_metaDataModel = MetaDataManager::instance()->createMetaDataModel(m_info.path(), !writable);
    }

    if(m_metaDataModel)
    {
        foreach(TagModel *tagModel, m_metaDataModel->tags())
        {
            TagEditor *editor = new TagEditor(tagModel, this);
            editor->setEnabled(!m_metaDataModel->isReadOnly());
            m_ui->tabWidget->addTab(editor, tagModel->name());
        }

        foreach(MetaDataItem item, m_metaDataModel->descriptions())
        {
            QTextEdit *textEdit = new QTextEdit(this);
            textEdit->setReadOnly(true);
            textEdit->setPlainText(item.value().toString());
            m_ui->tabWidget->addTab(textEdit, item.name());
        }
    }
    printInfo();
}

void DetailsDialog::printInfo()
{
    SoundCore *core = SoundCore::instance();
    QString formattedText, metaDataRows, streamInfoRows, propertyRows;
    QStringList tableParts;

    //tags
    metaDataRows += formatRow(tr("Title"), m_info.value(Qmmp::TITLE));
    metaDataRows += formatRow(tr("Artist"), m_info.value(Qmmp::ARTIST));
    metaDataRows += formatRow(tr("Album artist"), m_info.value(Qmmp::ALBUMARTIST));
    metaDataRows += formatRow(tr("Album"), m_info.value(Qmmp::ALBUM));
    metaDataRows += formatRow(tr("Comment"), m_info.value(Qmmp::COMMENT));
    metaDataRows += formatRow(tr("Genre"), m_info.value(Qmmp::GENRE));
    metaDataRows += formatRow(tr("Composer"), m_info.value(Qmmp::COMPOSER));
    metaDataRows += formatRow(tr("Year"), m_info.value(Qmmp::YEAR));
    metaDataRows += formatRow(tr("Track"), m_info.value(Qmmp::TRACK));
    metaDataRows += formatRow(tr("Disc number"), m_info.value(Qmmp::DISCNUMBER));
    metaDataRows = metaDataRows.trimmed();
    if(!metaDataRows.isEmpty())
        tableParts << metaDataRows;

    //stream information
    if(core->state() == Qmmp::Playing && core->path() == m_info.path())
    {
        foreach(QString key, core->streamInfo().keys())
            streamInfoRows += formatRow(key, core->streamInfo().value(key));
    }
    streamInfoRows = streamInfoRows.trimmed();
    if(!streamInfoRows.isEmpty())
        tableParts << streamInfoRows;

    //properties
    QList<MetaDataItem> items;
    if(m_info.duration() > 0)
        items << MetaDataItem(tr("Duration"), MetaDataFormatter::formatDuration(m_info.duration()));
    if(!m_metaDataModel || !(m_metaDataModel->dialogHints() & MetaDataModel::COMPLETE_PROPERTY_LIST))
    {
        items << MetaDataItem(tr("Bitrate"), m_info.value(Qmmp::BITRATE).toInt(), tr("kbps"));
        items << MetaDataItem(tr("Sample rate"), m_info.value(Qmmp::SAMPLERATE).toInt(), tr("Hz"));
        items << MetaDataItem(tr("Channels"), m_info.value(Qmmp::CHANNELS).toInt());
        items << MetaDataItem(tr("Sample size"), m_info.value(Qmmp::BITS_PER_SAMPLE).toInt(), tr("bits"));
        items << MetaDataItem(tr("Format name"), m_info.value(Qmmp::FORMAT_NAME));
        items << MetaDataItem(tr("File size"), m_info.value(Qmmp::FILE_SIZE).toInt() / 1024, tr("KiB"));
    }
    if(m_metaDataModel)
        items << m_metaDataModel->extraProperties();
    foreach (MetaDataItem item, items)
        propertyRows += formatRow(item);
    propertyRows = propertyRows.trimmed();
    if(!propertyRows.isEmpty())
        tableParts << propertyRows;

    //create table
    if(layoutDirection() == Qt::RightToLeft)
        formattedText.append("<DIV align=\"right\" dir=\"rtl\">");
    else
        formattedText.append("<DIV>");
    formattedText.append("<TABLE>");

    formattedText += tableParts.join("<tr><td colspan=2><hr></td></tr>");

    formattedText.append("</TABLE>");
    formattedText.append("</DIV>");
    m_ui->textEdit->setHtml(formattedText);
}

QString DetailsDialog::formatRow(const QString &key, const QString &value) const
{
    if(value.isEmpty() || key.isEmpty())
        return QString();
    QString str("<tr>");
    if(layoutDirection() == Qt::RightToLeft)
        str.append("<td>" + value + "</td> <td style=\"padding-left: 15px;\"><b>" + key + "</b></td>");
    else
        str.append("<td><b>" + key + "</b></td> <td style=\"padding-left: 15px;\">" + value + "</td>");
    str.append("</tr>");
    return str;
}

QString DetailsDialog::formatRow(const MetaDataItem &item) const
{
    if(item.value().isNull() || item.name().isEmpty() || !item.value().isValid())
        return QString();

    QString value;
    if(item.value().type() == QVariant::Bool)
        value = item.value().toBool() ? tr("Yes") : tr("No");
    else if(item.value().type() == QVariant::Double)
        value = QString("%1").arg(item.value().toDouble(), 0, 'f', 4);
    else
        value = item.value().toString();

    if(value.isEmpty() || value == "0" || value == "0.0000")
        return QString();

    if(!item.suffix().isEmpty())
        value += QLatin1String(" ") + item.suffix();

    return formatRow(item.name(), value);
}
