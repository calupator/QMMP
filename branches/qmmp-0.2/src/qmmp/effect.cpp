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
#include <QtGui>
#include <QStringList>
#include <QDir>
#include <QApplication>

#include "effectfactory.h"
#include "constants.h"
#include "effect.h"

Effect::Effect(QObject *parent)
        : QObject(parent)
{}

Effect::~Effect()
{}

void Effect::configure(ulong freq, int chan, int res)
{
    m_freq = freq;
    m_chan = chan;
    m_res = res;

}

ulong Effect::frequency()
{
    return m_freq;
}

int Effect::channels()
{
    return m_chan;
}

int Effect::resolution()
{
    return m_res;
}

static QList<EffectFactory*> *factories = 0;
static QStringList files;

static void checkFactories()
{
    if (! factories)
    {
        files.clear();
        factories = new QList<EffectFactory *>;

        QDir pluginsDir (qApp->applicationDirPath());
        pluginsDir.cdUp();
        pluginsDir.cd("./"LIB_DIR"/qmmp/Effect");
        foreach (QString fileName, pluginsDir.entryList(QDir::Files))
        {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            QObject *plugin = loader.instance();
            if (loader.isLoaded())
                qDebug("Effect: plugin loaded - %s", qPrintable(fileName));
            else
                qWarning("Effect: %s", qPrintable(loader.errorString ()));

            EffectFactory *factory = 0;
            if (plugin)
                factory = qobject_cast<EffectFactory *>(plugin);

            if (factory)
            {
                factories->append(factory);
                files << pluginsDir.absoluteFilePath(fileName);
                qApp->installTranslator(factory->createTranslator(qApp));
            }
        }
    }
}

QList<Effect*> Effect::create(QObject *parent)
{
    checkFactories();
    QList<Effect*> effects;
    EffectFactory *factory = 0;
    foreach (factory, *factories)
    {
        if(isEnabled(factory))
            effects.append(factory->create(parent));
    }
    return effects;
}

QList<EffectFactory*> *Effect::effectFactories()
{
    checkFactories();
    return factories;
}

QStringList Effect::effectFiles()
{
    checkFactories();
    return files;
}

void Effect::setEnabled(EffectFactory* factory, bool enable)
{
    checkFactories();
    if(!factories->contains(factory))
        return;

    QString name = files.at(factories->indexOf(factory)).section('/',-1);
    QSettings settings ( QDir::homePath() +"/.qmmp/qmmprc", QSettings::IniFormat );
    QStringList effList = settings.value("Effect/plugin_files").toStringList();

    if(enable)
    {
        if (!effList.contains(name))
            effList << name;
    }
    else
        effList.removeAll(name);
    settings.setValue("Effect/plugin_files", effList);
}

bool Effect::isEnabled(EffectFactory* factory)
{
    checkFactories();
    if(!factories->contains(factory))
        return FALSE;
    QString name = files.at(factories->indexOf(factory)).section('/',-1);
    QSettings settings ( QDir::homePath() +"/.qmmp/qmmprc", QSettings::IniFormat );
    QStringList effList = settings.value("Effect/plugin_files").toStringList();
    return effList.contains(name);
}

