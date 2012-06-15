/***************************************************************************
 *   Copyright (C) 1999 by Johan Levin                                     *
 *                                                                         *
 *   Copyright (C) 2011 by Ilya Kotov                                      *
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

#include <QSettings>
#include <math.h>
#include <stdlib.h>
#include <qmmp/qmmp.h>
#include "stereoplugin.h"

StereoPlugin *StereoPlugin::m_instance = 0;

StereoPlugin::StereoPlugin() : Effect()
{
    m_instance = this;
    m_format = Qmmp::PCM_S16LE;
    m_avg = 0;
    m_ldiff = 0;
    m_rdiff = 0;
    m_tmp = 0;
    m_mul = 2.0;
    m_chan = 0;
    QSettings settings(Qmmp::configFile(), QSettings::IniFormat);
    m_mul = settings.value("extra_stereo/intensity", 1.0).toDouble();
}

StereoPlugin::~StereoPlugin()
{
    m_instance = 0;
}

void StereoPlugin::applyEffect(Buffer *b)
{
    if(m_chan != 2)
        return;

    m_mutex.lock();
    if(m_format == Qmmp::PCM_S16LE)
    {
        short *data = (short *)b->data;
        for (uint i = 0; i < b->nbytes >> 1; i += 2)
        {
            m_avg = (data[i] + data[i + 1]) / 2;
            m_ldiff = data[i] - m_avg;
            m_rdiff = data[i + 1] - m_avg;

            m_tmp = m_avg + m_ldiff * m_mul;
            data[i] = qBound(-32768.0, m_tmp, 32767.0);
            m_tmp = m_avg + m_rdiff * m_mul;
            data[i + 1] = qBound(-32768.0, m_tmp, 32767.0);
        }
    }
    else if(m_format == Qmmp::PCM_S24LE || m_format == Qmmp::PCM_S32LE)
    {
        int *data = (int *)b->data;
        for (uint i = 0; i < b->nbytes >> 2; i += 2)
        {
            m_avg = (data[i] + data[i + 1]) / 2;
            m_ldiff = data[i] - m_avg;
            m_rdiff = data[i + 1] - m_avg;

            m_tmp = m_avg + m_ldiff * m_mul;
            data[i] = m_tmp;
            m_tmp = m_avg + m_rdiff * m_mul;
            data[i + 1] = m_tmp;
        }
    }
    m_mutex.unlock();
}

void StereoPlugin::configure(quint32 freq, int chan, Qmmp::AudioFormat format)
{
    m_chan = chan;
    m_format = format;
    Effect::configure(freq, chan, format);
}

void StereoPlugin::setIntensity(double level)
{
    m_mutex.lock();
    m_mul = level;
    m_mutex.unlock();
}

StereoPlugin* StereoPlugin::instance()
{
    return m_instance;
}
