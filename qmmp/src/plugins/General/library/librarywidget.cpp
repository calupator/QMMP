/***************************************************************************
 *   Copyright (C) 2020-2021 by Ilya Kotov                                 *
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

#include <QSettings>
#include <qmmp/qmmp.h>
#include "librarymodel.h"
#include "ui_librarywidget.h"
#include "librarywidget.h"

LibraryWidget::LibraryWidget(bool dialog, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::LibraryWidget)
{
    m_ui->setupUi(this);
    m_model = new LibraryModel(this);
    m_ui->treeView->setModel(m_model);

    if(dialog)
    {
        setWindowFlags(Qt::Dialog);
        setAttribute(Qt::WA_DeleteOnClose);
        setAttribute(Qt::WA_QuitOnClose, false);
    }
    else
    {
        m_ui->buttonBox->hide();
    }

    QSettings settings(Qmmp::configFile(), QSettings::IniFormat);
    m_ui->filterLineEdit->setVisible(settings.value("Library/quick_search_visible", true).toBool());
}

LibraryWidget::~LibraryWidget()
{
    delete m_ui;
}

void LibraryWidget::refresh()
{
    m_ui->filterLineEdit->clear();
    m_model->refresh();
}

void LibraryWidget::on_filterLineEdit_textChanged(const QString &text)
{
    m_model->setFilter(text);
    m_model->refresh();
    if(text.count() >= 3)
        m_ui->treeView->expandAll();
}
