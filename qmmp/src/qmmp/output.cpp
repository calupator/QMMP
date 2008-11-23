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
#include "buffer.h"
#include "output.h"
#include "volumecontrol.h"

#include <stdio.h>

Output::Output (QObject* parent) : QThread (parent), m_recycler (stackSize())
{
    m_handler = 0;
    m_frequency = 0;
    m_channels = 0;
    m_kbps = 0;
    m_totalWritten = 0;
    m_currentSeconds = -1;
    m_bytesPerSecond = 0;
    m_userStop = FALSE;
    m_pause = FALSE;
}

void Output::configure(quint32 freq, int chan, int prec)
{
    m_frequency = freq;
    m_channels = chan;
    m_precision = prec;
    m_bytesPerSecond = freq * chan * (prec / 8);
}

void Output::pause()
{
    m_pause = !m_pause;
    Qmmp::State state = m_pause ? Qmmp::Paused: Qmmp::Playing;
    dispatch(state);
}

void Output::stop()
{
    m_userStop = TRUE;
}

qint64 Output::written()
{
    return m_totalWritten;
}

void Output::seek(qint64 pos)
{
    m_totalWritten = pos * m_bytesPerSecond;
    m_currentSeconds = -1;
}

Recycler *Output::recycler()
{
    return &m_recycler;
}

QMutex *Output::mutex()
{
    return &m_mutex;
}

void Output::setStateHandler(StateHandler *handler)
{
    m_handler = handler;
}

Output::~Output()
{}

void Output::dispatchVisual (Buffer *buffer, unsigned long written,
                             int chan, int prec)
{
    if (!buffer)
        return;
    foreach (Visual *visual, *Visual::visuals())
    {
        visual->mutex()->lock ();
        visual->add ( buffer, written, chan, prec );
        visual->mutex()->unlock();
    }
}


void Output::clearVisuals()
{
    foreach (Visual *visual, *Visual::visuals())
    {
        visual->mutex()->lock ();
        visual->clear ();
        visual->mutex()->unlock();
    }
}

void Output::dispatch(qint64 elapsed,
                      qint64 totalTime,
                      int bitrate,
                      int frequency,
                      int precision,
                      int channels)
{
    if (m_handler)
        m_handler->dispatch(elapsed, totalTime, bitrate, frequency, precision, channels);
}

void Output::dispatch(const Qmmp::State &state)
{
    if (m_handler)
        m_handler->dispatch(state);
    if (state == Qmmp::Stopped)
        clearVisuals();
}

void Output::run()
{
    mutex()->lock ();
    if (!m_bytesPerSecond)
    {
        qWarning("Output: invalid audio parameters");
        mutex()->unlock ();
        return;
    }
    mutex()->unlock ();

    bool done = FALSE;
    Buffer *b = 0;
    qint64 l = 0;

    dispatch(Qmmp::Playing);

    while (!done)
    {
        mutex()->lock ();
        recycler()->mutex()->lock ();
        done = m_userStop;
        
        while (!done && (recycler()->empty() || m_pause))
        {
            mutex()->unlock();
            recycler()->cond()->wakeOne();
            recycler()->cond()->wait(recycler()->mutex());
            mutex()->lock ();
            done = m_userStop;
        }
        status();
        if (!b)
        {
            b = recycler()->next();
            if (b && b->rate)
                m_kbps = b->rate;
        }
        recycler()->cond()->wakeOne();
        recycler()->mutex()->unlock();
        mutex()->unlock();
        if (b)
        {
            dispatchVisual(b, m_totalWritten, m_channels, m_precision);
            changeVolume(b->data, b->nbytes, m_channels);
            if ((l = writeAudio(b->data, b->nbytes)) > 0)
                m_totalWritten += b->nbytes;
            else
                break;
        }
        mutex()->lock();
        //force buffer change
        recycler()->mutex()->lock ();
        recycler()->done();
        recycler()->mutex()->unlock();
        b = 0;
        mutex()->unlock();
    }

    mutex()->lock ();
    //write remaining data
    flush();
    dispatch(Qmmp::Stopped);
    mutex()->unlock();
}

void Output::status()
{
    long ct = (m_totalWritten - latency()) / m_bytesPerSecond;

    if (ct < 0)
        ct = 0;

    if (ct > m_currentSeconds)
    {
        m_currentSeconds = ct;
        dispatch(m_currentSeconds, m_totalWritten, m_kbps,
                 m_frequency, m_precision, m_channels);
    }
}

void Output::changeVolume(uchar *data, qint64 size, int chan)
{
    if (!SoftwareVolume::instance())
        return;
    if (chan > 1)
        for (qint64 i = 0; i < size/2; i+=2)
        {
            ((short*)data)[i]*= SoftwareVolume::instance()->left()/100.0;
            ((short*)data)[i+1]*= SoftwareVolume::instance()->right()/100.0;
        }
    else
    {
        int l = qMax(SoftwareVolume::instance()->left(), SoftwareVolume::instance()->right());
        for (qint64 i = 0; i < size/2; i++)
            ((short*)data)[i]*= l/100.0;
    }
}

// static methods

QList<OutputFactory*> *Output::m_factories = 0;
QStringList Output::m_files;
QTimer *Output::m_timer = 0;

void Output::checkFactories()
{
    if (!m_factories)
    {
        m_files.clear();
        m_factories = new QList<OutputFactory *>;

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
                m_files << pluginsDir.absoluteFilePath(fileName);
                qApp->installTranslator(factory->createTranslator(qApp));
            }
        }
    }
}

void Output::registerFactory ( OutputFactory *fact )
{
    m_factories->append ( fact );
}

Output *Output::create (QObject *parent)
{
    Output *output = 0;

    checkFactories();
    if (m_factories->isEmpty ())
    {
        qDebug("Output: unable to find output plugins");
        return output;
    }
    OutputFactory *fact = Output::currentFactory();
    if (!fact && !m_factories->isEmpty())
        fact = m_factories->at(0);
    if (fact)
        output = fact->create (parent);
    return output;
}

QList<OutputFactory*> *Output::outputFactories()
{
    checkFactories();
    return m_factories;
}

QStringList Output::outputFiles()
{
    checkFactories();
    return m_files;
}

void Output::setCurrentFactory(OutputFactory* factory)
{
    checkFactories();
    if (!m_factories->contains(factory))
        return;
    QSettings settings (QDir::homePath() +"/.qmmp/qmmprc", QSettings::IniFormat);
    settings.setValue ("Output/current_plugin", factory->properties().shortName);
}

OutputFactory *Output::currentFactory()
{
    checkFactories();
    QSettings settings (QDir::homePath() +"/.qmmp/qmmprc", QSettings::IniFormat);
#ifdef Q_OS_LINUX
    QString name = settings.value("Output/current_plugin", "alsa").toString();
#else
    QString name = settings.value("Output/current_plugin", "oss").toString();
#endif
    foreach(OutputFactory *factory, *m_factories)
    {
        if (factory->properties().shortName == name)
            return factory;
    }
    if (!m_factories->isEmpty())
        return m_factories->at(0);
    return 0;
}
