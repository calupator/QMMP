/***************************************************************************
 *   Copyright (C) 2006-2016 by Ilya Kotov                                 *
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
#ifndef DECODERVORBISFACTORY_H
#define DECODERVORBISFACTORY_H

#include <QObject>
#include <QString>
#include <QIODevice>
#include <QWidget>

#include <qmmp/decoder.h>
#include <qmmp/output.h>
#include <qmmp/decoderfactory.h>
#include <qmmp/fileinfo.h>
#include <qmmp/metadatamodel.h>

/**
    @author Ilya Kotov <forkotov02@ya.ru>
*/
class DecoderVorbisFactory : public QObject,
                          DecoderFactory
{
Q_OBJECT
Q_INTERFACES(DecoderFactory)

public:
    bool canDecode(QIODevice *input) const;
    const DecoderProperties properties() const;
    Decoder *create(const QString &path, QIODevice *input);
    MetaDataModel* createMetaDataModel(const QString &path, QObject *parent = 0);
    QList<FileInfo *> createPlayList(const QString &fileName, bool useMetaData, QStringList *);
    QObject* showDetails(QWidget *parent, const QString &path);
    void showSettings(QWidget *parent);
    void showAbout(QWidget *parent);
    QTranslator *createTranslator(QObject *parent);
};

#endif
