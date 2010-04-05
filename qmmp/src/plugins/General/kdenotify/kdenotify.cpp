/***************************************************************************
 *   Copyright (C) 2009-2010 by Artur Guzik                                *
 *   a.guzik88@gmail.com                                                   *
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

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QTimer>
#include <QImage>
#include <QApplication>
#include <QVariant>

#include <qmmpui/metadataformatter.h>
#include <qmmp/soundcore.h>
#include <qmmp/metadatamanager.h>

#include "kdenotify.h"

KdeNotify::KdeNotify(QObject *parent) : General(parent),m_useFreedesktopSpec(true)
{
    m_notifier = new QDBusInterface("org.freedesktop.Notifications",
                                  "/org/freedesktop/Notifications","org.freedesktop.Notifications",
                                  QDBusConnection::sessionBus(), this);
    if(m_notifier->lastError().type() != QDBusError::NoError)
    {
        delete(m_notifier);
        m_notifier = new QDBusInterface("org.kde.VisualNotifications",
                                      "/VisualNotifications", "org.kde.VisualNotifications",
                                      QDBusConnection::sessionBus(), this);
        if(m_notifier->lastError().type() != QDBusError::NoError)
        {
            qWarning() << "KdeNotify: Unable to create interface.";
            return;
        }
        m_useFreedesktopSpec = false;
    }
    qWarning() << "KdeNotify: DBus interfece created successfully.";
    QString path = QFileInfo(Qmmp::configFile()).absoluteDir().path();
    QDir dir(path);
    if(!dir.exists("kdenotifycache"))
        dir.mkdir("kdenotifycache");
    dir.cd("kdenotifycache");
    m_coverPath = dir.absolutePath() + "/cover.jpg";
    m_imagesDir = QDir(qApp->applicationFilePath () +"/../../share/qmmp/images").absolutePath();

    QSettings settings(Qmmp::configFile(),QSettings::IniFormat);
    settings.beginGroup("Kde_Notifier");
    m_notifyDuration = settings.value("notify_duration",5000).toInt();
    m_showCovers = settings.value("show_covers",true).toBool();
    m_template = settings.value("template", DEFAULT_TEMPLATE).toString();
    m_template.remove("\n");
    m_updateNotify = settings.value("update_notify",true).toBool();
    settings.endGroup();
    m_currentNotifyId = 0;

    if(m_updateNotify)
    {
        connect(SoundCore::instance(),SIGNAL(metaDataChanged()),SLOT(showMetaData()));
        connect(m_notifier,SIGNAL(NotificationClosed(uint,uint)),this,SLOT(notificationClosed(uint,uint)));
    }
    else
    {
        QTimer *timer = new QTimer(this);
        timer->setSingleShot(true);
        timer->setInterval(NOTIFY_DELAY); //after that notification will be showed.
        connect(timer,SIGNAL(timeout()),SLOT(showMetaData()));
        connect(SoundCore::instance(),SIGNAL(metaDataChanged()),timer, SLOT(start()));
    }
}

KdeNotify::~KdeNotify()
{
    QDir dir(QDir::home());
    dir.remove(m_coverPath);
}

QString KdeNotify::totalTimeString()
{
    int time = SoundCore::instance()->totalTime()/1000;

    if(time >= 3600)
    {
        return QString("%1:%2:%3").arg(time/3600,2,10,QChar('0')).arg(time%3600/60,2,10,QChar('0'))
                .arg(time%60,2,10,QChar('0'));
    }
    return QString("%1:%2").arg(time/60,2,10,QChar('0')).arg(time%60,2,10,QChar('0'));
}

QList<QVariant> KdeNotify::prepareNotification()
{
    SoundCore *core = SoundCore::instance();
    if(core->metaData(Qmmp::URL).isEmpty()) //prevent show empty notification
    {
        return QList<QVariant>();
    }
    QList<QVariant> args;
    args.append("Qmmp"); //app-name
    args.append(m_currentNotifyId); //replaces-id;
    if(!m_useFreedesktopSpec)
        args.append(""); //event-id
    args.append(m_imagesDir + "/app.png");  //app-icon(path to icon on disk)
    args.append(tr("Qmmp now playing:")); //summary (notification title)

    MetaDataFormatter f(m_template);
    QString body = f.parse(core->metaData(), core->totalTime()/1000);

    QString coverPath;
    if(m_showCovers)
    {
        QPixmap cover = MetaDataManager::instance()->getCover(core->metaData(Qmmp::URL));
        if(!cover.isNull())
        {
            coverPath = m_coverPath;
            cover.scaled(90,90,Qt::IgnoreAspectRatio,Qt::SmoothTransformation).save(coverPath);
        }
    }
    if(coverPath.isEmpty())
        coverPath = m_imagesDir + "/empty_cover.png";

    QString nBody;
    nBody.append(body);
    args.append(nBody); //body

    args.append(QStringList()); //actions

    QVariantMap hints;
    hints.insert("image_path",coverPath);
    args.append(hints); //hints

    args.append(m_notifyDuration); //timeout

    return args;
}

void KdeNotify::showMetaData()
{
    QList<QVariant> n = prepareNotification();
    if(!n.isEmpty())
    {
        QDBusReply<uint> reply = m_notifier->callWithArgumentList(QDBus::Block,"Notify",n);
        if(reply.isValid() && m_updateNotify)
        {
            m_currentNotifyId = reply.value();
        }
    }
}

void KdeNotify::notificationClosed(uint id, uint reason)
{
    Q_UNUSED(reason);
    qWarning() << "notificationClosed: " << id;
    if(m_currentNotifyId == id)
        m_currentNotifyId = 0;
}
