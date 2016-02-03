/**************************************************************************
*   Copyright (C) 20016 by Ilya Kotov                                     *
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

#ifndef CLASSICFILEDIALOGIMPL_H
#define CLASSICFILEDIALOGIMPL_H

#include "ui_classicfiledialog.h"
#include <QDialog>
#include <QCompleter>
#include <QAbstractItemView>
#include <qmmpui/filedialog.h>
#include <QFileSystemModel>


class ClassicFileDialogImpl : public QDialog
{
    Q_OBJECT
public:
    ClassicFileDialogImpl(QWidget *parent = 0);

    ~ClassicFileDialogImpl();

    void setModeAndMask(const QString &path, FileDialog::Mode m, const QStringList& mask = QStringList());
    QStringList selectedFiles ();

signals:
    void filesAdded(const QStringList&);

private slots:
    void updateDirSelection(const QItemSelection&s, const QItemSelection&);
    void updateFileSelection();
    void on_dirListView_doubleClicked(const QModelIndex&ind);

    void on_lookInComboBox_activated(const QString&);
    void on_upToolButton_clicked();
    void on_fileListView_doubleClicked(const QModelIndex&);
    void on_treeView_doubleClicked(const QModelIndex&);
    void on_fileNameLineEdit_textChanged (const QString &text);
    void on_addButton_clicked();
    void on_fileTypeComboBox_activated(int);


private:
    void updateFileList(const QString &path);
    void hideEvent (QHideEvent *event);
    int m_mode;
    QFileSystemModel* m_dirModel;

    Ui::ClassicFileDialog m_ui;

    void addToHistory(const QString &path);
    void addFiles(const QStringList &list);
    QStringList m_history;
    QStringList m_filters;

};

class PathCompleter : public QCompleter
{
    Q_OBJECT
public:
    PathCompleter(QAbstractItemModel *model, QAbstractItemView *itemView, QObject *parent = 0) : QCompleter(model, parent)
    {
        m_itemView = itemView;
    }

    QString pathFromIndex(const QModelIndex &index) const
    {
        const QFileSystemModel *fileModel = static_cast<const QFileSystemModel *>(model());
        QString currentLocation = fileModel->filePath(m_itemView->rootIndex());
        QString path = fileModel->filePath(index);
        if (path.startsWith(currentLocation))
        {
            path = path.mid(currentLocation.length() + 1);
        }
        return path;
    }


    QStringList splitPath(const QString &path) const
    {
        if (path.isEmpty())
            return QStringList(completionPrefix());
        QStringList parts;
        if (!path.startsWith(QDir::separator()))
        {
            const QFileSystemModel *fileModel = static_cast<const QFileSystemModel *>(model());
            QString currentLocation = QDir::toNativeSeparators(fileModel->filePath(m_itemView->rootIndex()));
            parts = QCompleter::splitPath(currentLocation);
        }
        parts << QCompleter::splitPath(path);
        return parts;
    }
private:
    QAbstractItemView *m_itemView;
};


#endif //CLASSICFILEDIALOGIMPL_H
