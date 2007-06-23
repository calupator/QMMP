
// Copyright (c) 2000-2001 Brad Hughes <bhughes@trolltech.com>
//
// Use, modification and distribution is allowed without limitation,
// warranty, or liability of any kind.
//

#ifndef   __output_h
#define   __output_h

class Output;

#include <QObject>
#include <QThread>
#include <QEvent>
#include <QList>
#include <QIODevice>
#include <outputfactory.h>

#include "recycler.h"

class QTimer;

class Visualization;


class OutputState
{
public:

    enum Type { Playing, Buffering, Info, Paused, Stopped, Volume, Error };

    OutputState()
            : m_type(Stopped),  m_error_msg(0), m_elasped_seconds(0),
              m_written_bytes(0), m_brate(0), m_freq(0), m_prec(0), m_chan(0),
              m_left(0), m_right(0)
    {}

    OutputState(Type t)
            : m_type(t),  m_error_msg(0), m_elasped_seconds(0),
              m_written_bytes(0), m_brate(0), m_freq(0), m_prec(0), m_chan(0),
              m_left(0), m_right(0)
    {}
    OutputState(long s, unsigned long w, int b, int f, int p, int c)
            : m_type(Info), m_error_msg(0), m_elasped_seconds(s),
              m_written_bytes(w), m_brate(b), m_freq(f), m_prec(p), m_chan(c),
              m_left(0), m_right(0)
    {}
    OutputState(int L, int R)
            : m_type(Volume), m_error_msg(0), m_elasped_seconds(0),
              m_written_bytes(0), m_brate(0), m_freq(0), m_prec(0), m_chan(0),
              m_left(L), m_right(R)
    {}
    OutputState(const QString &e)
            : m_type(Error), m_elasped_seconds(0), m_written_bytes(0),
              m_brate(0), m_freq(0), m_prec(0), m_chan(0),
              m_left(0), m_right(0)
    {
         m_error_msg = new QString(e);
    }
    ~OutputState()
    {
       if (m_error_msg)
           delete m_error_msg;
    }

    const QString *errorMessage() const { return m_error_msg; }

    const long &elapsedSeconds() const { return m_elasped_seconds; }
    const unsigned long &writtenBytes() const { return m_written_bytes; }
    const int &bitrate() const { return m_brate; }
    const int &frequency() const { return m_freq; }
    const int &precision() const { return m_prec; }
    const int &channels() const { return m_chan; }
    const Type &type() const { return m_type; }
    const int leftVolume() const { return m_left; }
    const int rightVolume() const { return m_right; }

private:
    Type m_type;
    QString *m_error_msg;
    long m_elasped_seconds;
    unsigned long m_written_bytes;
    int m_brate, m_freq, m_prec, m_chan;
    int m_left, m_right;    //volume
};




class Output : public QThread
{
    Q_OBJECT
public:

    enum VolumeType { Standard, Custom, Disabled };

    Output(QObject * parent = 0, VolumeType vt = Standard);
    ~Output();

    Recycler *recycler()
    {
        return &r;
    }

    void addVisual(Visualization *);
    void removeVisual(Visualization *);

    QMutex *mutex()
    {
        return &mtx;
    }

    VolumeType volumeControl(){return m_vol; };

    // abstract
    virtual bool isInitialized() const = 0;
    virtual bool initialize() = 0;
    virtual void uninitialize() = 0;
    virtual void configure(long, int, int, int) = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual long written() = 0;
    virtual long latency() = 0;
    virtual void seek(long) = 0;
    virtual void setVolume(int, int) {};

    static void registerFactory(OutputFactory *);
    static Output *create(QObject *);
    static QList<OutputFactory*> *outputFactories();
    static QStringList outputFiles();

public slots:
    virtual void checkVolume() {};

signals:
    void stateChanged(const OutputState&);

protected:
    void dispatch(OutputState::Type);
    void dispatch(long s, unsigned long w, int b, int f, int p, int c);
    void dispatch(const OutputState&);
    void dispatchVolume(int L, int R);
    void error(const QString &e);
    void dispatchVisual(Buffer *, unsigned long, int, int);
    void prepareVisuals();

private:
    QMutex mtx;
    Recycler r;
    QList<Visualization *> visuals;
    VolumeType m_vol;
};


#endif // __output_h
