// Copyright (c) 2000-2001 Brad Hughes <bhughes@trolltech.com>
//
// Use, modification and distribution is allowed without limitation,
// warranty, or liability of any kind.
//

#include <QtGui>
#include <QObject>
#include <QStringList>
#include <QApplication>
#include <QTimer>

#include "constants.h"
#include "output.h"

#include <stdio.h>

// static methods

static QList<OutputFactory*> *factories = 0;
static QStringList files;
static QTimer *timer = 0;

static void checkFactories()
{
    if ( ! factories )
    {
        files.clear();
        factories = new QList<OutputFactory *>;

        QDir pluginsDir ( qApp->applicationDirPath() );
        pluginsDir.cdUp();
        pluginsDir.cd ( "./"LIB_DIR"/qmmp/Output" );
        foreach ( QString fileName, pluginsDir.entryList ( QDir::Files ) )
        {
            QPluginLoader loader ( pluginsDir.absoluteFilePath ( fileName ) );
            QObject *plugin = loader.instance();
            if ( loader.isLoaded() )
                qDebug ( "Output: plugin loaded - %s", qPrintable ( fileName ) );
            else
                qWarning("Output: %s", qPrintable(loader.errorString ()));

            OutputFactory *factory = 0;
            if ( plugin )
                factory = qobject_cast<OutputFactory *> ( plugin );

            if ( factory )
            {
                Output::registerFactory ( factory );
                files << pluginsDir.absoluteFilePath(fileName);
            }
        }
    }
}

void Output::registerFactory ( OutputFactory *fact )
{
    factories->append ( fact );
}

Output *Output::create (QObject *parent)
{
    Output *output = 0;

    checkFactories();
    if (factories->isEmpty ())
    {
        qDebug("Output: unable to find output plugins");
        return output;
    }
    OutputFactory *fact = 0;
    foreach(fact, *factories)
    {
        if (isEnabled(fact))
            break;
        else
            fact = factories->at(0);
    }
    QSettings settings (QDir::homePath() +"/.qmmp/qmmprc", QSettings::IniFormat);
    bool useVolume = !settings.value("Volume/software_volume", FALSE).toBool();
    if (fact)
    {
        output = fact->create (parent, useVolume);
        if (useVolume)
        {
            timer = new QTimer(output);
            connect(timer, SIGNAL(timeout()), output, SLOT(checkVolume()));
            timer->start(125);
        }
        else
        {
            QTimer::singleShot(125, output, SLOT(checkSoftwareVolume()));
        }
    }
    return output;
}

QList<OutputFactory*> *Output::outputFactories()
{
    checkFactories();
    return factories;
}

QStringList Output::outputFiles()
{
    checkFactories();
    return files;
}

void Output::setEnabled(OutputFactory* factory)
{
    checkFactories();
    if (!factories->contains(factory))
        return;

    QString name = files.at(factories->indexOf(factory)).section('/',-1);
    QSettings settings (QDir::homePath() +"/.qmmp/qmmprc", QSettings::IniFormat);
    settings.setValue ("Output/plugin_file", name);
}

bool Output::isEnabled(OutputFactory* factory)
{
    checkFactories();
    if (!factories->contains(factory))
        return FALSE;
    QString name = files.at(factories->indexOf(factory)).section('/',-1);
    QSettings settings (QDir::homePath() +"/.qmmp/qmmprc", QSettings::IniFormat);
#ifdef Q_OS_LINUX
    return name == settings.value("Output/plugin_file", "libalsa.so").toString();
#else
    return name == settings.value("Output/plugin_file", "liboss.so").toString();
#endif
}

Output::Output (QObject* parent) : QThread (parent), r (stackSize())
{
    qRegisterMetaType<OutputState>("OutputState");
    m_bl = -1;
    m_br = -1;
}


Output::~Output()
{
    foreach(Visual *visual, m_vis_map.values ())
    {
        visual->setOutput(0);
        visual->close();
    }
    foreach (Visual *visual , visuals)  //external
    {
        visual->setOutput(0);
    }
}

void Output::error ( const QString &e )
{
    emit stateChanged ( OutputState ( e ) );
}

void Output::addVisual ( Visual *v )
{
    if (visuals.indexOf (v) == -1)
    {
        visuals.append (v);
        v->setOutput(this);
        qDebug("Output: added external visualization");
    }
}


void Output::removeVisual (Visual *v)
{
    v->setOutput(0);
    visuals.removeAll (v);
    if (m_vis_map.key(v))
    {
        VisualFactory *factory = m_vis_map.key(v);
        m_vis_map.remove(factory);
    }
}

void Output::processCloseEvent(Visual *v, QCloseEvent *event)
{
    if (m_vis_map.key(v))
    {
        VisualFactory *factory = m_vis_map.key(v);
        m_vis_map.remove(factory);
        if(event->spontaneous())
        {
            Visual::setEnabled(factory, FALSE);
            dispatch(OutputState::VisualRemoved);
        }
    }
}

void Output::addVisual(VisualFactory *factory, QWidget *parent)
{
    if (m_vis_map.value(factory))
        return;
    Visual::setEnabled(factory, TRUE);
    Visual* visual = factory->create(parent);
    visual->setWindowFlags(Qt::Window);
    if (visual)
    {
        visual->setOutput(this);
        qDebug("Output: added visual factory: %s",
               qPrintable(factory->properties().name));
        m_vis_map.insert (factory, visual);
        visual->show();
    }
}

void Output::removeVisual(VisualFactory *factory)
{
    if (m_vis_map.value(factory))
    {
        m_vis_map.value(factory)->close();
        m_vis_map.remove (factory);
    }
    Visual::setEnabled(factory, FALSE);
}

void Output::dispatchVisual ( Buffer *buffer, unsigned long written,
                              int chan, int prec )
{
    if (!buffer)
        return;
    Visual* visual = 0;
    foreach (visual , visuals)  //external
    {
        visual->mutex()->lock ();
        visual->add ( buffer, written, chan, prec );
        visual->mutex()->unlock();
    }
    foreach (visual , m_vis_map.values ())  //internal
    {
        visual->mutex()->lock ();
        visual->add ( buffer, written, chan, prec );
        visual->mutex()->unlock();
    }
}


void Output::clearVisuals()
{
    Visual *visual = 0;
    foreach (visual, visuals )
    {
        visual->mutex()->lock ();
        visual->clear();
        visual->mutex()->unlock();
    }
    foreach(visual, m_vis_map.values ())
    {
        visual->mutex()->lock ();
        visual->clear();
        visual->mutex()->unlock();
    }
}

void Output::dispatch(OutputState::Type st)
{
    if (st == OutputState::Stopped)
        clearVisuals();
    emit stateChanged ( OutputState(st) );
}

void Output::dispatch(long s, unsigned long w, int b, int f, int p, int c)
{
    emit stateChanged ( OutputState(s, w, b, f, p, c) );
}

void Output::dispatch ( const OutputState &st )
{
    if (st.type() == OutputState::Stopped)
        clearVisuals();
    emit stateChanged ( st );
}

void Output::dispatchVolume(int L, int R)
{
    emit stateChanged ( OutputState(L, R) );
}

void Output::checkVolume()
{
    int ll = 0, lr = 0;
    volume(&ll,&lr);
    ll = (ll > 100) ? 100 : ll;
    lr = (lr > 100) ? 100 : lr;
    ll = (ll < 0) ? 0 : ll;
    lr = (lr < 0) ? 0 : lr;
    if (m_bl!=ll || m_br!=lr)
    {
        m_bl = ll;
        m_br = lr;
        dispatchVolume(ll,lr);
    }
}

void Output::checkSoftwareVolume()
{
    QSettings settings(QDir::homePath()+"/.qmmp/qmmprc", QSettings::IniFormat);
    int L = settings.value("Volume/left", 80).toInt();
    int R = settings.value("Volume/right", 80).toInt();
    dispatchVolume(L, R);
}

