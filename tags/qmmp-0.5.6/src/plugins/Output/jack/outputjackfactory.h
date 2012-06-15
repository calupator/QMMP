/***************************************************************************
 *   Copyright (C) 2006-2008 by Ilya Kotov                                 *
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
#ifndef OUTPUTJACKFACTORY_H
#define OUTPUTJACKFACTORY_H


#include <QObject>
#include <QString>
#include <QIODevice>
#include <QWidget>

#include <qmmp/output.h>
#include <qmmp/outputfactory.h>


class OutputJACKFactory : public QObject,
                          OutputFactory
{
Q_OBJECT
Q_INTERFACES(OutputFactory);

public:
    const OutputProperties properties() const;
    Output* create(QObject* parent);
    VolumeControl *createVolumeControl(QObject *parent);
    void showSettings(QWidget* parent);
    void showAbout(QWidget *parent);
    QTranslator *createTranslator(QObject *parent);

};

#endif
