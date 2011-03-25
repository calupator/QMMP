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

#ifndef CUEMETADATAMODEL_H
#define CUEMETADATAMODEL_H

#include <qmmp/metadatamodel.h>

class CUEParser;

class CUEMetaDataModel : public MetaDataModel
{
Q_OBJECT
public:
    CUEMetaDataModel(const QString &url, QObject *parent);
    ~CUEMetaDataModel();
    QHash<QString, QString> audioProperties();
    QPixmap cover();
    QString coverPath();

private:
    CUEParser *m_parser;
    QString m_path;
};

#endif // CUEMETADATAMODEL_H
