/***************************************************************************
 *   Copyright (C) 2009 by Ilya Kotov                                      *
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

#ifndef METADATAMODEL_H
#define METADATAMODEL_H

#include <QHash>
#include <QList>
#include <QString>
#include <QObject>
#include <QPixmap>
#include "tagmodel.h"

class MetaDataModel : public QObject
{
Q_OBJECT
public:
    MetaDataModel(QObject *parent = 0);
    virtual ~MetaDataModel();
    virtual QHash<QString, QString> audioProperties();
    virtual QHash<QString, QString> descriptions();
    virtual QList<TagModel* > tags();
    virtual QPixmap cover();
    virtual QString coverPath();
};

#endif // METADATAMODEL_H
