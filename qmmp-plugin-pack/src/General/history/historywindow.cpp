/***************************************************************************
 *   Copyright (C) 2017 by Ilya Kotov                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <QStringList>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QSettings>
#include <QProgressBar>
#include <QTreeWidgetItem>
#include <qmmp/qmmp.h>
#include "historywindow.h"
#include "ui_historywindow.h"

HistoryWindow::HistoryWindow(QSqlDatabase db, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::HistoryWindow)
{
    m_ui->setupUi(this);
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);
    m_db = db;
    readSettings();

    QDateTime t = QDateTime::currentDateTime();
    t.setTime(QTime(23, 59, 0 ,0));
    m_ui->toDateEdit->setDateTime(t);
    t.setTime(QTime(0, 0, 0, 0));
    t = t.addDays(-7);
    m_ui->fromDateEdit->setDateTime(t);
    on_executeButton_clicked();
}

HistoryWindow::~HistoryWindow()
{
    delete m_ui;
}

void HistoryWindow::loadHistory()
{
    m_ui->historyTreeWidget->clear();

    if(!m_db.isOpen())
        return;

    QSqlQuery query(m_db);

    query.prepare("SELECT Timestamp,Title,Artist,AlbumArtist,Album,Comment,Genre,Composer,Track,Year,Duration,URL "
                  "FROM track_history WHERE Timestamp BETWEEN :from and :to");
    query.bindValue(":from", m_ui->fromDateEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":to", m_ui->toDateEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss"));

    if(!query.exec())
    {
        qWarning("HistoryWindow: query error: %s", qPrintable(query.lastError().text()));
        return;
    }

    QColor bgColor = palette().color(QPalette::Highlight);
    QColor textColor = palette().color(QPalette::HighlightedText);

    while (query.next())
    {
        FileInfo info;
        info.setMetaData(Qmmp::TITLE,  query.value(1).toString());
        info.setMetaData(Qmmp::ARTIST,  query.value(2).toString());
        info.setMetaData(Qmmp::ALBUMARTIST,  query.value(3).toString());
        info.setMetaData(Qmmp::ALBUM,  query.value(4).toString());
        info.setMetaData(Qmmp::COMMENT,  query.value(5).toString());
        info.setMetaData(Qmmp::GENRE,  query.value(6).toString());
        info.setMetaData(Qmmp::COMPOSER,  query.value(7).toString());
        info.setMetaData(Qmmp::TRACK,  query.value(8).toString());
        info.setMetaData(Qmmp::YEAR,  query.value(9).toString());
        info.setLength(query.value(10).toInt() / 1000);
        info.setPath(query.value(11).toString());

        QDateTime dateTime = QDateTime::fromString(query.value(0).toString(), "yyyy-MM-dd hh:mm:ss");
        dateTime.setTimeSpec(Qt::UTC);
        QString dateStr = dateTime.toLocalTime().toString(tr("dd MMMM yyyy"));
        QString timeStr = dateTime.toLocalTime().toString(tr("hh:mm:ss"));
        int topLevelCount = m_ui->historyTreeWidget->topLevelItemCount();

        if(!topLevelCount)
        {
            m_ui->historyTreeWidget->addTopLevelItem(new QTreeWidgetItem());
            m_ui->historyTreeWidget->topLevelItem(topLevelCount++)->setText(0, dateStr);
            m_ui->historyTreeWidget->topLevelItem(topLevelCount - 1)->setFirstColumnSpanned(true);
            m_ui->historyTreeWidget->topLevelItem(topLevelCount - 1)->setTextAlignment(0, Qt::AlignCenter);
            m_ui->historyTreeWidget->topLevelItem(topLevelCount - 1)->setBackgroundColor(0, bgColor);
            m_ui->historyTreeWidget->topLevelItem(topLevelCount - 1)->setTextColor(0, textColor);
        }
        else if(m_ui->historyTreeWidget->topLevelItem(topLevelCount - 1)->text(0) != dateStr)
        {
            m_ui->historyTreeWidget->addTopLevelItem(new QTreeWidgetItem());
            m_ui->historyTreeWidget->topLevelItem(topLevelCount++)->setText(0, dateStr);
            m_ui->historyTreeWidget->topLevelItem(topLevelCount - 1)->setFirstColumnSpanned(true);
            m_ui->historyTreeWidget->topLevelItem(topLevelCount - 1)->setTextAlignment(0, Qt::AlignCenter);
            m_ui->historyTreeWidget->topLevelItem(topLevelCount - 1)->setBackgroundColor(0, bgColor);
            m_ui->historyTreeWidget->topLevelItem(topLevelCount - 1)->setTextColor(0, textColor);
        }

        QTreeWidgetItem *topLevelItem = m_ui->historyTreeWidget->topLevelItem(topLevelCount - 1);
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, timeStr);
        item->setText(1, m_formatter.format(info.metaData(), info.length()));
        topLevelItem->addChild(item);
    }

    m_ui->historyTreeWidget->expandAll();
}

void HistoryWindow::loadDistribution()
{
    m_ui->distributionTreeWidget->clear();

    if(!m_db.isOpen())
        return;

    QSqlQuery query(m_db);

    query.prepare("SELECT max(c) FROM( SELECT count(*) as c FROM track_history WHERE Timestamp BETWEEN :from and :to "
                  "GROUP BY date(Timestamp, 'localtime'))");
    query.bindValue(":from", m_ui->fromDateEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":to", m_ui->toDateEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss"));
    if(!query.exec())
    {
        qWarning("HistoryWindow: query error: %s", qPrintable(query.lastError().text()));
        return;
    }
    if(!query.next())
    {
        qWarning("HistoryWindow: empty result");
        return;
    }
    int maxCount = query.value(0).toInt();
    qDebug("%d", maxCount);
    query.finish();

    query.prepare("SELECT count(*), date(Timestamp, 'localtime') FROM track_history WHERE Timestamp BETWEEN :from and :to "
                  "GROUP BY date(Timestamp, 'localtime')");
    query.bindValue(":from", m_ui->fromDateEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":to", m_ui->toDateEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss"));
    if(!query.exec())
    {
        qWarning("HistoryWindow: query error: %s", qPrintable(query.lastError().text()));
        return;
    }

    QColor bgColor = palette().color(QPalette::Highlight);
    QColor textColor = palette().color(QPalette::HighlightedText);

    while (query.next())
    {
        QDate date = QDate::fromString(query.value(1).toString(), "yyyy-MM-dd");
        QString monthStr = date.toString(tr("MM-yyyy"));
        QString dayStr = date.toString(tr("dd MMMM"));
        int topLevelCount = m_ui->distributionTreeWidget->topLevelItemCount();

        if(!topLevelCount)
        {
            m_ui->distributionTreeWidget->addTopLevelItem(new QTreeWidgetItem());
            m_ui->distributionTreeWidget->topLevelItem(topLevelCount++)->setText(0,  monthStr);
            m_ui->distributionTreeWidget->topLevelItem(topLevelCount - 1)->setFirstColumnSpanned(true);
            m_ui->distributionTreeWidget->topLevelItem(topLevelCount - 1)->setTextAlignment(0, Qt::AlignCenter);
            m_ui->distributionTreeWidget->topLevelItem(topLevelCount - 1)->setBackgroundColor(0, bgColor);
            m_ui->distributionTreeWidget->topLevelItem(topLevelCount - 1)->setTextColor(0, textColor);
        }
        else if(m_ui->distributionTreeWidget->topLevelItem(topLevelCount - 1)->text(0) != monthStr)
        {
            m_ui->distributionTreeWidget->addTopLevelItem(new QTreeWidgetItem());
            m_ui->distributionTreeWidget->topLevelItem(topLevelCount++)->setText(0, monthStr);
            m_ui->distributionTreeWidget->topLevelItem(topLevelCount - 1)->setFirstColumnSpanned(true);
            m_ui->distributionTreeWidget->topLevelItem(topLevelCount - 1)->setTextAlignment(0, Qt::AlignCenter);
            m_ui->distributionTreeWidget->topLevelItem(topLevelCount - 1)->setBackgroundColor(0, bgColor);
            m_ui->distributionTreeWidget->topLevelItem(topLevelCount - 1)->setTextColor(0, textColor);
        }

        QTreeWidgetItem *topLevelItem = m_ui->distributionTreeWidget->topLevelItem(topLevelCount - 1);
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, dayStr);
        topLevelItem->addChild(item);
        QProgressBar *countBar = new QProgressBar();
        countBar->setMaximum(maxCount);
        countBar->setFormat("%v");
        countBar->setValue(query.value(0).toInt());
        m_ui->distributionTreeWidget->setItemWidget(item, 1, countBar);
    }

    m_ui->distributionTreeWidget->expandAll();
}

void HistoryWindow::loadTopSongs()
{
    m_ui->topSongsTreeWidget->clear();

    if(!m_db.isOpen())
        return;

    QSqlQuery query(m_db);

    query.prepare("SELECT count(*) as c,Timestamp,Title,Artist,AlbumArtist,Album,Comment,Genre,Composer,Track,Year,Duration,URL "
                  "FROM track_history WHERE Timestamp BETWEEN :from and :to "
                  "GROUP BY Artist,Title ORDER BY c DESC LIMIT 100");
    query.bindValue(":from", m_ui->fromDateEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":to", m_ui->toDateEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss"));

    if(!query.exec())
    {
        qWarning("HistoryWindow: query error: %s", qPrintable(query.lastError().text()));
        return;
    }

    int maxCount = 0;

    while (query.next())
    {
        FileInfo info;
        info.setMetaData(Qmmp::TITLE,  query.value(2).toString());
        info.setMetaData(Qmmp::ARTIST,  query.value(3).toString());
        info.setMetaData(Qmmp::ALBUMARTIST,  query.value(4).toString());
        info.setMetaData(Qmmp::ALBUM,  query.value(5).toString());
        info.setMetaData(Qmmp::COMMENT,  query.value(6).toString());
        info.setMetaData(Qmmp::GENRE,  query.value(7).toString());
        info.setMetaData(Qmmp::COMPOSER,  query.value(8).toString());
        info.setMetaData(Qmmp::TRACK,  query.value(9).toString());
        info.setMetaData(Qmmp::YEAR,  query.value(10).toString());
        info.setLength(query.value(11).toInt() / 1000);
        info.setPath(query.value(12).toString());

        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, m_formatter.format(info.metaData(), info.length()));

        m_ui->topSongsTreeWidget->addTopLevelItem(item);

        if(!maxCount)
            maxCount = query.value(0).toInt();

        QProgressBar *countBar = new QProgressBar();
        countBar->setMaximum(maxCount);
        countBar->setFormat("%v");
        countBar->setValue(query.value(0).toInt());
        m_ui->topSongsTreeWidget->setItemWidget(item, 1, countBar);
    }
}

void HistoryWindow::loadTopArtists()
{
    m_ui->topArtistsTreeWidget->clear();

    if(!m_db.isOpen())
        return;

    QSqlQuery query(m_db);

    query.prepare("SELECT count(*) as c,Artist "
                  "FROM track_history WHERE Timestamp BETWEEN :from and :to "
                  "GROUP BY Artist ORDER BY c DESC LIMIT 100");
    query.bindValue(":from", m_ui->fromDateEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":to", m_ui->toDateEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss"));

    if(!query.exec())
    {
        qWarning("HistoryWindow: query error: %s", qPrintable(query.lastError().text()));
        return;
    }

    int maxCount = 0;

    while (query.next())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, query.value(1).toString());

        m_ui->topArtistsTreeWidget->addTopLevelItem(item);

        if(!maxCount)
            maxCount = query.value(0).toInt();

        QProgressBar *countBar = new QProgressBar();
        countBar->setMaximum(maxCount);
        countBar->setFormat("%v");
        countBar->setValue(query.value(0).toInt());
        m_ui->topArtistsTreeWidget->setItemWidget(item, 1, countBar);
    }
}

void HistoryWindow::loadTopGenres()
{
    m_ui->topGenresTreeWidget->clear();

    if(!m_db.isOpen())
        return;

    QSqlQuery query(m_db);

    query.prepare("SELECT count(*) as c,Genre "
                  "FROM track_history WHERE Timestamp BETWEEN :from and :to "
                  "GROUP BY Genre ORDER BY c DESC LIMIT 100");
    query.bindValue(":from", m_ui->fromDateEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":to", m_ui->toDateEdit->dateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss"));

    if(!query.exec())
    {
        qWarning("HistoryWindow: query error: %s", qPrintable(query.lastError().text()));
        return;
    }

    int maxCount = 0;

    while (query.next())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, query.value(1).toString());

        m_ui->topGenresTreeWidget->addTopLevelItem(item);

        if(!maxCount)
            maxCount = query.value(0).toInt();

        QProgressBar *countBar = new QProgressBar();
        countBar->setMaximum(maxCount);
        countBar->setFormat("%v");
        countBar->setValue(query.value(0).toInt());
        m_ui->topGenresTreeWidget->setItemWidget(item, 1, countBar);
    }
}

void HistoryWindow::readSettings()
{
    QSettings settings(Qmmp::configFile(), QSettings::IniFormat);
    settings.beginGroup("History");
    restoreGeometry(settings.value("geometry").toByteArray());
    m_ui->historyTreeWidget->header()->restoreState(settings.value("history_state").toByteArray());
    m_ui->distributionTreeWidget->header()->restoreState(settings.value("distribution_state").toByteArray());
    m_ui->topSongsTreeWidget->header()->restoreState(settings.value("top_songs_state").toByteArray());
    m_ui->topArtistsTreeWidget->header()->restoreState(settings.value("top_artists_state").toByteArray());
    m_ui->topGenresTreeWidget->header()->restoreState(settings.value("top_genres_state").toByteArray());
    m_formatter.setPattern(settings.value("title_format", "%if(%p,%p - %t,%t)").toString());
    settings.endGroup();
}

void HistoryWindow::closeEvent(QCloseEvent *)
{
    QSettings settings(Qmmp::configFile(), QSettings::IniFormat);
    settings.beginGroup("History");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("history_state", m_ui->historyTreeWidget->header()->saveState());
    settings.setValue("distribution_state", m_ui->distributionTreeWidget->header()->saveState());
    settings.setValue("top_songs_state", m_ui->topSongsTreeWidget->header()->saveState());
    settings.setValue("top_artists_state", m_ui->topArtistsTreeWidget->header()->saveState());
    settings.setValue("top_genres_state", m_ui->topGenresTreeWidget->header()->saveState());
    settings.endGroup();
}

void HistoryWindow::on_executeButton_clicked()
{
    loadHistory();
    loadDistribution();
    loadTopSongs();
    loadTopArtists();
    loadTopGenres();
}
