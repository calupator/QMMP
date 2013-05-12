/***************************************************************************
 *   Copyright (C) 2013 by Ilya Kotov                                      *
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

#ifndef QMMPPLUGINCACHE_P_H
#define QMMPPLUGINCACHE_P_H

#include <QString>
#include <QObject>
#include <QSettings>

class DecoderFactory;
class OutputFactory;

/*! @internal
 * @author Ilya Kotov <forkotov02@hotmail.ru>
 */
class QmmpPluginCache
{
public:
    QmmpPluginCache(const QString &file, QSettings *settings);

    const QString shortName() const;
    const QString file() const;
    int priority() const;
    bool hasError() const;

    DecoderFactory *decoderFactory();
    OutputFactory *outputFactory();


    //EngineFactory *engineFactory();

    static void cleanup(QSettings *settings);

private:
    QObject *instance();
    QString m_path;
    QString m_shortName;
    bool m_error;
    QObject *m_instance;
    DecoderFactory *m_decoderFactory;
    OutputFactory *m_outputFactory;
    int m_priority;
};

#endif // QMMPPLUGINCACHE_P_H
