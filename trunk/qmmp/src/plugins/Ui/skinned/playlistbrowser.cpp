/***************************************************************************
 *   Copyright (C) 2009-2013 by Ilya Kotov                                 *
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

#include <QAction>
#include <QApplication>
#include <QStyle>
#include <QKeyEvent>
#include <qmmpui/playlistmanager.h>
#include "playlistbrowser.h"

PlayListBrowser::PlayListBrowser(PlayListManager *manager, QWidget *parent) : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowModality(Qt::NonModal);
    ui.setupUi(this);
    m_pl_manager = manager;
    connect(m_pl_manager, SIGNAL(playListsChanged()), SLOT(updateList()));
    connect(ui.newButton, SIGNAL(clicked()), m_pl_manager, SLOT(createPlayList()));
    updateList();
    //actions
    QAction *renameAct = new QAction(tr("Rename"), this);
    QAction *removeAct = new QAction(tr("Delete"), this);
    connect(renameAct,SIGNAL(triggered()), SLOT(rename()));
    connect(removeAct,SIGNAL(triggered()), SLOT(on_deleteButton_clicked()));
    ui.listWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui.listWidget->addAction(renameAct);
    ui.listWidget->addAction(removeAct);
    ui.downButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowDown));
    ui.upButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowUp));
    ui.newButton->setIcon(QIcon::fromTheme("document-new"));
    ui.deleteButton->setIcon(QIcon::fromTheme("edit-delete"));
}

PlayListBrowser::~PlayListBrowser()
{}

void PlayListBrowser::updateList()
{
    ui.listWidget->clear();
    foreach(PlayListModel *model, m_pl_manager->playLists())
        ui.listWidget->addItem(model->name());
    ui.listWidget->setCurrentRow (m_pl_manager->selectedPlayListIndex());
    //mark current playlist
    int current = m_pl_manager->currentPlayListIndex();
    QListWidgetItem *item = ui.listWidget->item(current);
    if(item)
    {
        QFont font = item->font();
        font.setBold(true);
        item->setFont(font);
    }
}

void PlayListBrowser::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    m_pl_manager->activatePlayList(ui.listWidget->row(item));
}

void PlayListBrowser::on_listWidget_itemChanged(QListWidgetItem *item)
{
    m_pl_manager->playListAt(ui.listWidget->row(item))->setName(item->text());
}

void PlayListBrowser::on_listWidget_itemPressed (QListWidgetItem *item)
{
    m_pl_manager->selectPlayList(ui.listWidget->row(item));
}

void PlayListBrowser::rename()
{
    QListWidgetItem *item = ui.listWidget->currentItem();
    if(item)
    {
        item->setFlags(Qt::ItemIsEditable | item->flags());
        ui.listWidget->editItem(item);
    }
}

void PlayListBrowser::on_deleteButton_clicked()
{
    QList <PlayListModel *> models;
    foreach(QListWidgetItem *item, ui.listWidget->selectedItems())
        models.append(m_pl_manager->playListAt(ui.listWidget->row (item)));
    foreach(PlayListModel *model, models)
        m_pl_manager->removePlayList(model);
}

void PlayListBrowser::on_downButton_clicked()
{
    int pos = m_pl_manager->indexOf(m_pl_manager->selectedPlayList());
    if(pos < m_pl_manager->count() - 1)
        m_pl_manager->move(pos, pos + 1);
}

void PlayListBrowser::on_upButton_clicked()
{
    int pos = m_pl_manager->indexOf(m_pl_manager->selectedPlayList());
    if(pos > 0)
        m_pl_manager->move(pos, pos - 1);
}

void PlayListBrowser::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Return)
    {
        QListWidgetItem *item = ui.listWidget->currentItem();
        if(item)
        {
            disconnect(m_pl_manager, SIGNAL(playListsChanged()), this, SLOT(updateList()));
            m_pl_manager->activatePlayList(ui.listWidget->row(item));
            m_pl_manager->selectPlayList(ui.listWidget->row(item));
            connect(m_pl_manager, SIGNAL(playListsChanged()), SLOT(updateList()));
            updateList();
        }
        e->accept();
    }
    else
        QDialog::keyPressEvent(e);
}
