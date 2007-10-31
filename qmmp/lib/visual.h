/***************************************************************************
 *   Copyright (C) 2007 by Ilya Kotov                                      *
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
#ifndef VISUAL_H
#define VISUAL_H

/**
   @author Ilya Kotov <forkotov02@hotmail.ru>
*/

#include <QMutex>
#include <QStringList>
#include <QWidget>
#include <QMap>

class Buffer;
class Decoder;
class Output;
class VisualFactory;

class Visual : public QWidget
{
    Q_OBJECT
public:
    Visual(QWidget *parent);

    virtual ~Visual();

    virtual void add(Buffer *, unsigned long, int, int) = 0;
    virtual void clear() = 0;

    Decoder *decoder() const;
    void setDecoder(Decoder *decoder);
    Output *output() const;
    void setOutput(Output *output);
    QMutex *mutex();

    //static methods
    static QList<VisualFactory*> *visualFactories();
    static QStringList visualFiles();
    static void setEnabled(VisualFactory* factory, bool enable = TRUE);
    static bool isEnabled(VisualFactory* factory);

protected:
    virtual void closeEvent (QCloseEvent *);

private:
    Decoder *m_decoder;
    Output *m_output;
    QMutex m_mutex;
};

#endif
