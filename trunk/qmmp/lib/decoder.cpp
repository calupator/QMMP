// Copyright (c) 2000-2001 Brad Hughes <bhughes@trolltech.com>
//
// Use, modification and distribution is allowed without limitation,
// warranty, or liability of any kind.
//
#include <QtGui>
#include <QObject>
#include <QStringList>
#include <QApplication>
#include <QSettings>


#include "constants.h"
#include "buffer.h"
#include "output.h"
#include "visualization.h"
#include "decoderfactory.h"
#include "streamreader.h"
extern "C"{
#include "equ/iir.h" 
}
#include "decoder.h"


Decoder::Decoder(QObject *parent, DecoderFactory *d, QIODevice *i, Output *o)
        : QThread(parent), fctry(d), in(i), out(o), blksize(0),m_eqInited(FALSE),
        m_useEQ(FALSE)
{
    out->recycler()->clear();
    int b[] = {0,0,0,0,0,0,0,0,0,0};
    setEQ(b, 0);
    qRegisterMetaType<DecoderState>("DecoderState");
}

Decoder::~Decoder()
{
    fctry = 0;
    in = 0;
    out = 0;
    blksize = 0;
}

// static methods

static QList<DecoderFactory*> *factories = 0;
static QStringList files;
static QStringList blacklist;

static void checkFactories()
{
    QSettings settings ( QDir::homePath() +"/.qmmp/qmmprc", QSettings::IniFormat );
    blacklist = settings.value("Decoder/disabled_plugins").toStringList ();
    if (! factories)
    {
        files.clear();
        factories = new QList<DecoderFactory *>;

        QDir pluginsDir (qApp->applicationDirPath());
        pluginsDir.cdUp();
        pluginsDir.cd("lib/qmmp/Input");
        foreach (QString fileName, pluginsDir.entryList(QDir::Files))
        {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            QObject *plugin = loader.instance();
            if (loader.isLoaded())
            {
                qDebug("Decoder: plugin loaded - %s", qPrintable(fileName));
            }
            DecoderFactory *factory = 0;
            if (plugin)
                factory = qobject_cast<DecoderFactory *>(plugin);

            if (factory)
            {
                factories->append(factory);
                files << pluginsDir.absoluteFilePath(fileName);
            }
        }
        //remove physically deleted plugins from blacklist
        QStringList names;
        foreach (QString filePath, files)
        {
            names.append(filePath.section('/',-1));
        }
        int i = 0;
        while (i < blacklist.size())
        {
            if (!names.contains(blacklist.at(i)))
                blacklist.removeAt(i);
            else
                i++;
        }
        settings.setValue("Decoder/disabled_plugins",blacklist);
    }
}


QStringList Decoder::all()
{
    checkFactories();

    QStringList l;
    DecoderFactory *fact;
    foreach(fact, *factories)
    {
        l << fact->properties().description;
    }
    return l;
}

QStringList Decoder::decoderFiles()
{
    checkFactories();
    return files;
}


bool Decoder::supports(const QString &source)
{
    checkFactories();

    for (int i=0; i<factories->size(); ++i)
    {
        if (factories->at(i)->supports(source) &&
                !blacklist.contains(files.at(i).section('/',-1)))
        {
            return TRUE;
        }
    }
    return FALSE;
}

Decoder *Decoder::create(QObject *parent, const QString &source,
                         QIODevice *input,
                         Output *output)
{
    Decoder *decoder = 0;
    qDebug(qPrintable(source));
    DecoderFactory *fact = 0;

    if(!input->open(QIODevice::ReadOnly))
    {
        qDebug("Decoder: cannot open input");
        return decoder;
    } 
    StreamReader* sreader = qobject_cast<StreamReader *>(input);
    if(sreader)
    {
        fact = Decoder::findByMime(sreader->contentType());
        if(!fact)
            fact = Decoder::findByContent(sreader);
    }
    else
        fact = Decoder::findByPath(source);

    if (fact)
    {
        decoder = fact->create(parent, input, output);
    }
    if(!decoder)
        input->close();

    return decoder;
}

DecoderFactory *Decoder::findByPath(const QString& source)
{
    checkFactories();

    for (int i=0; i<factories->size(); ++i)
    {
        if (factories->at(i)->supports(source) &&
                !blacklist.contains(files.at(i).section('/',-1)))
        {
            return factories->at(i);
        }
    }
    qDebug("Decoder: unable to find factory by path");
    return 0;
}

DecoderFactory *Decoder::findByMime(const QString& type)
{
    checkFactories();
    for (int i=0; i<factories->size(); ++i)
    {
        if (!blacklist.contains(files.at(i).section('/',-1)))
        {
            QStringList types = factories->at(i)->properties().contentType.split(";");
            for(int j=0; j<types.size(); ++j)
            {
                if(type == types[j] && !types[j].isEmpty())
                    return factories->at(i);
            }
        }
    }
    qDebug("Decoder: unable to find factory by mime");
    return 0;
}

DecoderFactory *Decoder::findByContent(QIODevice *input)
{
    checkFactories();

    for (int i=0; i<factories->size(); ++i)
    {
        if (factories->at(i)->canDecode(input) &&
                !blacklist.contains(files.at(i).section('/',-1)))
        {
            return factories->at(i);
        }
    }
    qDebug("Decoder: unable to find factory by content");
    return 0;
}

FileTag *Decoder::createTag(const QString& source)
{
    DecoderFactory *fact = Decoder::findByPath(source);
    if (fact)
    {
        return fact->createTag(source);
    }
    return 0;
}

QString Decoder::filter()
{
    QString allflt(tr("All Supported Bitstreams ("));
    QString flt;

    checkFactories();
    DecoderFactory *fact;
    for (int i = 0; i<factories->size(); ++i)
    {
        if (!blacklist.contains(files.at(i).section('/',-1)))
        {
            fact = (*factories)[i];
            allflt +=fact->properties().filter.toLower() +" ";
            flt += fact->properties().description + " (" + fact->properties().filter + ")";
            flt += ";;";
        }
    }
    if (!flt.isEmpty ())
        flt = flt.left(flt.size ()-2);

    allflt += ");;";

    return allflt + flt;
}

QStringList Decoder::nameFilters()
{
    checkFactories();
    QStringList filters;
    for (int i=0; i<factories->size(); ++i)
    {
        if (!blacklist.contains(files.at(i).section('/',-1)))
            filters << factories->at(i)->properties().filter.split(" ", QString::SkipEmptyParts);
    }
    return filters;
}

QList<DecoderFactory*> *Decoder::decoderFactories()
{
    checkFactories();
    return factories;
}

void Decoder::dispatch(const DecoderState &st)
{
    emit stateChanged(st);
}

void Decoder::dispatch(DecoderState::Type st)
{
    emit stateChanged(DecoderState(st));
}

void Decoder::error(const QString &e)
{
    emit stateChanged(DecoderState(e));
}

ulong Decoder::produceSound(char *data, ulong output_bytes, ulong bitrate, int nch)
{
    ulong sz = output_bytes < blksize ? output_bytes : blksize;
    Buffer *b = output()->recycler()->get();

    if (!m_eqInited)
    {
        init_iir();
        m_eqInited = TRUE;
    }
    if (m_useEQ)
    {
        iir((void*) data,sz,nch);
    }
    memcpy(b->data, data, sz);

    if (sz != blksize)
        memset(b->data + sz, 0, blksize - sz);

    b->nbytes = blksize;
    b->rate = bitrate;

    output()->recycler()->add();

    output_bytes -= sz;
    memmove(data, data + sz, output_bytes);
    return sz;
}

void Decoder::setEQ(int bands[10], int preamp)
{
    set_preamp(0, 1.0 + 0.0932471 *preamp + 0.00279033 * preamp * preamp);
    set_preamp(1, 1.0 + 0.0932471 *preamp + 0.00279033 * preamp * preamp);
    for (int i=0; i<10; ++i)
    {
        int value = bands[i];
        set_gain(i,0, 0.03*value+0.000999999*value*value);
        set_gain(i,1, 0.03*value+0.000999999*value*value);
    }
}

