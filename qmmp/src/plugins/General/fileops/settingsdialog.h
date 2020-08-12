/***************************************************************************
 *   Copyright (C) 2009-2020 by Ilya Kotov                                 *
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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ui_settingsdialog.h"

/**
    @author Ilya Kotov <forkotov02@ya.ru>
*/
class ActionItem;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent = nullptr);

    ~SettingsDialog();


public slots:
    virtual void accept() override;

private slots:
    void on_newButton_clicked();
    void on_deleteButton_clicked();
    void updateLineEdits();
    void on_destinationEdit_textChanged(QString dest);
    void on_patternEdit_textChanged(QString pattern);
    void addTitleString(const QString &str);
    void on_destButton_clicked();
    void on_tableWidget_itemDoubleClicked (QTableWidgetItem *item);

private:
    void createMenus();
    Ui::SettingsDialog m_ui;
};

class ActionItem: public QTableWidgetItem
{
public:
    explicit ActionItem (const QString &text): QTableWidgetItem(text){}

    inline const QString &pattern() const
    {
        return m_pattern;
    }

    inline const QString &destination() const
    {
        return m_destination;
    }

    inline void setPattern(const QString &pattern)
    {
        m_pattern = pattern;
    }

    inline void setDestination(const QString &dest)
    {
        m_destination = dest;
    }

private:
    QString m_pattern, m_destination;
};


#endif
