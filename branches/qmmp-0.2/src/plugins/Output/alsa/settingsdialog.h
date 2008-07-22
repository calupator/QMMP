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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

extern "C"{
#include <alsa/asoundlib.h> 
}

#include "ui_settingsdialog.h"

/**
	@author Ilya Kotov <forkotov02@hotmail.ru>
*/
class SettingsDialog : public QDialog
{
Q_OBJECT
public:
    SettingsDialog(QWidget *parent = 0);

    ~SettingsDialog();

private slots:
    void setText(int);
    void writeSettings();
    void showMixerDevices(int);

private:
    Ui::SettingsDialog ui;
    void getCards();
    void getCardDevices(int card);
    void getMixerDevices(QString card);
    int getMixer(snd_mixer_t **mixer, QString card);
    QStringList m_devices;
    QList <QString> m_cards;

};

#endif
