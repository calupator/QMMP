/***************************************************************************
 *   Copyright (C) 2006-2013 by Ilya Kotov                                 *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         **
 ***************************************************************************/
#include <QPixmap>
#include <QResizeEvent>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QSettings>
#include <QMenu>
#include <QUrl>
#include <QApplication>
#include <QHelpEvent>
#include <QTimer>
#include <QScrollBar>
#include <qmmpui/playlistitem.h>
#include <qmmpui/playlistmodel.h>
#include <qmmpui/mediaplayer.h>
#include <qmmpui/playlistmanager.h>
#include "popupwidget.h"
#include "listwidget.h"

#define INVALID_ROW -1

ListWidget::ListWidget(PlayListModel *model, QWidget *parent): QWidget(parent)
{
    m_update = false;
    m_popupWidget = 0;
    m_menu = 0;
    m_scroll_direction = NONE;
    m_prev_y = 0;
    m_anchor_row = INVALID_ROW;
    m_player = MediaPlayer::instance();
    connect (m_player, SIGNAL(repeatableChanged(bool)), SLOT(updateList()));
    m_first = 0;
    m_rows = 0;
    m_scroll = false;
    m_select_on_release = false;
    readSettings();
    setAcceptDrops(true);
    setMouseTracking(true);
    m_timer = new QTimer(this);
    m_timer->setInterval(50);
    connect(m_timer, SIGNAL(timeout()), SLOT(autoscroll()));
    m_model = model;
    connect (m_model, SIGNAL(currentChanged()), SLOT(recenterCurrent()));
    connect (m_model, SIGNAL(listChanged()), SLOT(updateList()));
    m_scrollBar = new QScrollBar(Qt::Vertical, this);
    connect(m_scrollBar, SIGNAL(valueChanged (int)), SLOT(scroll(int)));
}

ListWidget::~ListWidget()
{}

void ListWidget::readSettings()
{
    QSettings settings(Qmmp::configFile(), QSettings::IniFormat);
    settings.beginGroup("Simple");
    m_font.fromString(settings.value("pl_font", QApplication::font().toString()).toString());
    m_extra_font = m_font;
    m_extra_font.setPointSize(m_font.pointSize() - 1);
    m_show_protocol = settings.value ("pl_show_protocol", false).toBool();
    m_show_number = settings.value ("pl_show_numbers", true).toBool();
    m_align_numbres = settings.value ("pl_align_numbers", false).toBool();
    m_show_anchor = settings.value("pl_show_anchor", false).toBool();
    bool show_popup = settings.value("pl_show_popup", false).toBool();

    if (m_update)
    {
        delete m_metrics;
        delete m_extra_metrics;
        m_metrics = new QFontMetrics(m_font);
        m_font.setBold(true);
        m_bold_metrics = new QFontMetrics(m_font);
        m_font.setBold(false);
        m_extra_metrics = new QFontMetrics(m_extra_font);
        m_rows = height() / (m_metrics->lineSpacing() + 2);
        updateList();
        if(m_popupWidget)
        {
            m_popupWidget->deleteLater();
            m_popupWidget = 0;
        }
    }
    else
    {
        m_update = true;
        m_metrics = new QFontMetrics(m_font);
        m_font.setBold(true);
        m_bold_metrics = new QFontMetrics(m_font);
        m_font.setBold(false);
        m_extra_metrics = new QFontMetrics(m_extra_font);
    }
    if(show_popup)
        m_popupWidget = new PlayListPopup::PopupWidget(this);

}

void ListWidget::paintEvent(QPaintEvent *)
{
    m_normal = palette().color(QPalette::Text);
    m_current = palette().color(QPalette::Text);
    m_highlighted = palette().color(QPalette::HighlightedText);
    m_normal_bg = palette().color(QPalette::Base);
    m_selected_bg = palette().color(QPalette::Highlight);


    QPainter painter(this);
    //m_painter.setPen(Qt::white);
    painter.setFont(m_font);
    painter.setBrush(QBrush(m_normal_bg));
#if QT_VERSION >= 0x040700
    painter.setLayoutDirection(Qt::LayoutDirectionAuto);
#endif
    bool rtl = (layoutDirection() == Qt::RightToLeft);
    int x = width() - (m_scrollBar->isVisible() ? m_scrollBar->sizeHint().width() : 0);
    int sx = 0, sy = 0;

    painter.drawRect(-1,-1, x + 1, height()+1);

    for (int i = 0; i < m_titles.size(); ++i)
    {
        if(i % 2 == 0)
        {
            painter.setBrush(QBrush(palette().color(QPalette::AlternateBase)));
            painter.setPen(palette().color(QPalette::AlternateBase));
            painter.drawRect (6, i * (m_metrics->lineSpacing() + 2),
                                x - 10, m_metrics->lineSpacing() + 1);
        }
        else
        {
            painter.setPen(m_normal_bg);
            painter.setBrush(QBrush(m_normal_bg));
            painter.drawRect (6,  i * (m_metrics->lineSpacing() + 2),
                                x - 10, m_metrics->lineSpacing() + 1);
        }

        if (m_show_anchor && i == m_anchor_row - m_first)
        {
            painter.setBrush(m_model->isSelected(i + m_first) ? m_selected_bg : m_normal_bg);
            painter.setPen(m_normal);
            painter.drawRect (6, i * (m_metrics->lineSpacing() + 2),
                                x - 10, m_metrics->lineSpacing() + 1);
        }
        else
        {
            if (m_model->isSelected(i + m_first))
            {
                painter.setBrush(QBrush(m_selected_bg));
                painter.setPen(m_selected_bg);
                painter.drawRect (6, i * (m_metrics->lineSpacing() + 2),
                                    x - 10, m_metrics->lineSpacing() + 1);
            }
        }

        QFontMetrics *metrics = 0;
        if (m_model->currentRow() == i + m_first)
        {
            m_font.setBold(true);
            metrics = m_bold_metrics;
            painter.setPen(m_current);
        }
        else
        {
            m_font.setBold(false);
            metrics = m_metrics;
        }

        painter.setFont(m_font);

        if (m_model->isSelected(i + m_first))
            painter.setPen(m_highlighted);
        else
            painter.setPen(m_normal);

        sy = (i + 1) * (2 + m_metrics->lineSpacing()) - 2 - m_metrics->descent();

        if(m_number_width)
        {
            QString number = QString("%1").arg(m_first+i+1);
            sx = 10 + m_number_width - metrics->width(number);
            if(rtl)
                sx = x - sx - metrics->width(number);
            painter.drawText(sx, sy, number);

            sx = 10 + m_number_width + metrics->width("9");
            if(rtl)
                sx = x - sx - metrics->width(m_titles.at(i));
            painter.drawText(sx, sy, m_titles.at(i));
        }
        else
        {
            sx = rtl ? x - 10 - metrics->width(m_titles.at(i)) : 10;
            painter.drawText(sx, sy, m_titles.at(i));
        }

        QString extra_string = getExtraString(m_first + i);
        if(!extra_string.isEmpty())
        {
            painter.setFont(m_extra_font);

            if(m_times.at(i).isEmpty())
            {
                sx = rtl ? 7 : x - 7 - m_extra_metrics->width(extra_string);
                painter.drawText(sx, sy, extra_string);
            }
            else
            {
                sx = x - 10 - m_extra_metrics->width(extra_string) - metrics->width(m_times.at(i));
                if(rtl)
                    sx = x - sx - m_extra_metrics->width(extra_string);
                painter.drawText(sx, sy, extra_string);
            }
            painter.setFont(m_font);
        }
        sx = rtl ? 7 : x - 7 - metrics->width(m_times.at(i));
        painter.drawText(sx, sy, m_times.at(i));
    }
    //draw line
    if(m_number_width)
    {
        painter.setPen(m_normal);
        sx = rtl ? x - 10 - m_number_width - m_metrics->width("9")/2 - 1 :
                   10 + m_number_width + m_metrics->width("9")/2 - 1;
        painter.drawLine(sx, 2, sx, sy);
    }
}

void ListWidget::mouseDoubleClickEvent (QMouseEvent *e)
{
    int y = e->y();
    int row = rowAt(y);
    if (INVALID_ROW != row)
    {
        m_model->setCurrent(row);
        MediaPlayer::instance()->playListManager()->selectPlayList(m_model);
        MediaPlayer::instance()->playListManager()->activatePlayList(m_model);
        MediaPlayer::instance()->stop();
        MediaPlayer::instance()->play();
        emit selectionChanged();
        update();
    }
}


void ListWidget::mousePressEvent(QMouseEvent *e)
{
    if(m_popupWidget)
        m_popupWidget->hide();
    m_scroll = true;
    int y = e->y();
    int row = rowAt(y);

    if (INVALID_ROW != row && m_model->count() > row)
    {
        m_pressed_row = row;
        if(e->button() == Qt::RightButton && !m_model->isSelected(row))
        {
            m_model->clearSelection();
            m_model->setSelected(row, true);
            m_anchor_row = m_pressed_row;
            QWidget::mousePressEvent(e);
            return;
        }

        if (m_model->isSelected(row) && (e->modifiers() == Qt::NoModifier))
        {
            m_select_on_release = true;
            QWidget::mousePressEvent(e);
            return;
        }

        if ((Qt::ShiftModifier & e->modifiers()))
        {
            bool select = true;
            if (m_pressed_row > m_anchor_row)
            {
                for (int j = m_anchor_row;j <= m_pressed_row;j++)
                {
                    m_model->setSelected(j, select);
                }
            }
            else
            {
                for (int j = m_anchor_row;j >= m_pressed_row;j--)
                {
                    m_model->setSelected(j, select);
                }
            }
            m_anchor_row = m_pressed_row;
        }
        else //ShiftModifier released
        {
            if ((Qt::ControlModifier & e->modifiers()))
            {
                m_model->setSelected(row, !m_model->isSelected(row));
            }
            else //ControlModifier released
            {
                m_model->clearSelection();
                m_model->setSelected(row, true);
            }
            m_anchor_row = m_pressed_row;
        }
        update();
    }
    QWidget::mousePressEvent(e);
}

void ListWidget::resizeEvent(QResizeEvent *e)
{
    m_scrollBar->setGeometry(width() - m_scrollBar->sizeHint().width(), 0,
                             m_scrollBar->sizeHint().width(), height());

    m_rows = e->size().height() / (m_metrics->lineSpacing() + 2);

    //m_scroll = true;
    recenterCurrent();
    updateList();
    QWidget::resizeEvent(e);
}

void ListWidget::wheelEvent (QWheelEvent *e)
{
    if (m_model->count() <= m_rows)
        return;
    if ((m_first == 0 && e->delta() > 0) ||
            ((m_first == m_model->count() - m_rows) && e->delta() < 0))
        return;
    m_first -= e->delta()/40;  //40*3 TODO: add step to config
    if (m_first < 0)
        m_first = 0;

    if (m_first > m_model->count() - m_rows)
        m_first = m_model->count() - m_rows;

    m_scroll = false;
    updateList();
}

bool ListWidget::event (QEvent *e)
{
    if(m_popupWidget)
    {
        if(e->type() == QEvent::ToolTip)
        {
            QHelpEvent *helpEvent = (QHelpEvent *) e;
            int row = rowAt(helpEvent->y());
            if(row < 0)
            {
                m_popupWidget->deactivate();
                return QWidget::event(e);
            }
            e->accept();
            m_popupWidget->prepare(m_model->item(row), helpEvent->globalPos());
            return true;
        }
        else if(e->type() == QEvent::Leave)
            m_popupWidget->deactivate();
    }
    return QWidget::event(e);
}

void ListWidget::updateList()
{
    if (m_model->count() < (m_rows+m_first+1) && m_rows< m_model->count())
    {
        m_first = m_model->count() - m_rows;
    }
    if (m_model->count() < m_rows + 1)
    {
        m_first = 0;
        m_scrollBar->setMaximum(0);
        m_scrollBar->setValue(0);
        emit positionChanged(0,0);
    }
    else
    {
        m_scrollBar->setMaximum(m_model->count() - m_rows);
        m_scrollBar->setValue(m_first);
        emit positionChanged(m_first, m_model->count() - m_rows);
    }
    if (m_model->count() <= m_first)
    {
        m_first = 0;
        m_scrollBar->setMaximum(qMax(0, m_model->count() - m_rows));
        m_scrollBar->setValue(0);
        emit positionChanged(0, qMax(0, m_model->count() - m_rows));
    }

    m_titles = m_model->getTitles(m_first, m_rows);
    m_times  = m_model->getTimes(m_first, m_rows);
    m_scroll = false;
    //add numbers
    for (int i = 0; i < m_titles.size() && m_show_number && !m_align_numbres; ++i)
    {
        QString title = m_titles.at(i);
        m_titles.replace(i, title.prepend(QString("%1").arg(m_first+i+1)+". "));

    }
    //song numbers width
    if(m_show_number && m_align_numbres && m_model->count())
    {
        m_number_width = m_metrics->width("9") * QString::number(m_model->count()).size();
    }
    else
        m_number_width = 0;

    //elide title
    QString extra_string;
    for (int i=0; i<m_titles.size(); ++i)
    {
        extra_string = getExtraString(m_first + i);
        int extra_string_space = extra_string.isEmpty() ? 0 : m_metrics->width(extra_string);
        if(m_number_width)
            extra_string_space += m_number_width + m_metrics->width("9");
        m_titles.replace(i, m_metrics->elidedText (m_titles.at(i), Qt::ElideRight,
                                                   width() -  m_metrics->width(m_times.at(i)) - 22 - extra_string_space));
    }

    m_scrollBar->setVisible(m_model->count() > m_rows);

    update();
}

void ListWidget::autoscroll()
{
    SimpleSelection sel = m_model->getSelection(m_pressed_row);
    if ((sel.m_top == 0 && m_scroll_direction == TOP && sel.count() > 1) ||
            (sel.m_bottom == m_model->count() - 1 && m_scroll_direction == DOWN && sel.count() > 1))
        return;

    if(m_scroll_direction == DOWN)
    {
        int row = m_first + m_rows;
        (m_first + m_rows < m_model->count()) ? m_first ++ : m_first;
        m_model->moveItems(m_pressed_row,row);
        m_pressed_row = row;
    }
    else if(m_scroll_direction == TOP && m_first > 0)
    {
        m_first --;
        m_model->moveItems(m_pressed_row, m_first);
        m_pressed_row = m_first;
    }
}

void ListWidget::scroll(int sc)
{
    if (m_model->count() <= m_rows)
        return;
    m_first = sc;
    m_scroll = true;
    updateList();
}

void ListWidget::updateSkin()
{
    //loadColors();
    update();
}

void ListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}


void ListWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        QList<QUrl> list_urls = event->mimeData()->urls();
        event->acceptProposedAction();
        QApplication::restoreOverrideCursor();

        foreach(QUrl u, list_urls)
        {
            if(u.scheme() == "file")
                m_model->add(QFileInfo(u.toLocalFile()).absoluteFilePath());
            else
                m_model->add(u.toString());
        }
    }
}

const QString ListWidget::getExtraString(int i)
{
    QString extra_string;

    if (m_show_protocol && m_model->item(i)->url().contains("://"))
        extra_string = "[" + m_model->item(i)->url().split("://").at(0) + "]";

    if (m_model->isQueued(m_model->item(i)))
    {
        int index = m_model->queuedIndex(m_model->item(i));
        extra_string += "|"+QString::number(index + 1)+"|";
    }

    if(m_model->currentRow() == i && m_player->isRepeatable())
        extra_string += "|R|";
    else if(m_model->isStopAfter(m_model->item(i)))
        extra_string += "|S|";

    extra_string = extra_string.trimmed(); //remove white space
    if(!extra_string.isEmpty())
        extra_string.prepend(" ");
    return extra_string;
}

void ListWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() == Qt::LeftButton)
    {
        m_scroll = true;
        if (m_prev_y > e->y())
            m_scroll_direction = TOP;
        else if (m_prev_y < e->y())
            m_scroll_direction = DOWN;
        else
            m_scroll_direction = NONE;

        if(e->y() < 0 || e->y() > height())
        {
            if(!m_timer->isActive())
                m_timer->start();
            return;
        }
        m_timer->stop();

        int row = rowAt(e->y());

        if (INVALID_ROW != row)
        {
            SimpleSelection sel = m_model->getSelection(m_pressed_row);
            if(sel.count() > 1 && m_scroll_direction == TOP)
            {
                if(sel.m_top == 0 || sel.m_top == m_first)
                    return;
            }
            else if(sel.count() > 1 && m_scroll_direction == DOWN)
            {
                if(sel.m_bottom == m_model->count() - 1 || sel.m_bottom == m_first + m_rows)
                    return;
            }
            m_model->moveItems(m_pressed_row,row);

            m_prev_y = e->y();
            m_scroll = false;
            m_pressed_row = row;
        }
    }
    else if(m_popupWidget)
    {
        int row = rowAt(e->y());
        if(row < 0 || m_popupWidget->url() != m_model->item(row)->url())
            m_popupWidget->deactivate();
    }
}

void ListWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (false != m_select_on_release)
    {
        m_model->clearSelection();
        m_model->setSelected(m_pressed_row,true);
        //if(e->modifiers() != Qt::ShiftModifier)
        m_anchor_row = m_pressed_row;
        m_select_on_release = false;
    }
    m_pressed_row = INVALID_ROW;
    m_scroll_direction = NONE;
    m_timer->stop();
    m_scroll = false;
    QWidget::mouseReleaseEvent(e);
}

int ListWidget::rowAt(int y) const
{
    for (int i = 0; i < qMin(m_rows, m_model->count() - m_first); ++i)
    {
        if ((y >= i * (m_metrics->lineSpacing() + 2)) && (y <= (i+1) * (m_metrics->lineSpacing() + 2)))
            return m_first + i;
    }
    return INVALID_ROW;
}

void ListWidget::contextMenuEvent(QContextMenuEvent * event)
{
    if (menu())
        menu()->exec(event->globalPos());
}

void ListWidget::recenterCurrent()
{
    //qDebug("%d", m_rows);
    if (!m_scroll)
    {
        if (m_first + m_rows < m_model->currentRow() + 1)
            m_first = qMin(m_model->count() - m_rows,
                           m_model->currentRow() - m_rows/2 + 1);
        else if (m_first > m_model->currentRow())
            m_first = qMax (m_model->currentRow() - m_rows/2 + 1, 0);
    }
}
