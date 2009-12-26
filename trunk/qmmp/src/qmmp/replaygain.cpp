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

#include <math.h>
#include "replaygain.h"

ReplayGain::ReplayGain()
{
    m_bits = 0;
    m_scale = 1.0;
}

void ReplayGain::setSampleSize(int bits)
{
    m_bits = bits;
    updateScale();
}

void ReplayGain::setReplayGainInfo(const QMap<Qmmp::ReplayGainKey, double> &info)
{
    m_info = info;
    updateScale();
    if(m_settings.mode() != ReplayGainSettings::DISABLED)
    {
        qDebug("ReplayGain: track: gain=%f dB, peak=%f; album: gain=%f dB, peak=%f",
               m_info[Qmmp::REPLAYGAIN_TRACK_GAIN],
               m_info[Qmmp::REPLAYGAIN_TRACK_PEAK],
               m_info[Qmmp::REPLAYGAIN_ALBUM_GAIN],
               m_info[Qmmp::REPLAYGAIN_ALBUM_PEAK]);
        qDebug("ReplayGain: scale=%f", m_scale);
    }
    else
        qDebug("ReplayGain: disabled");
}

void ReplayGain::setReplayGainSettings(const ReplayGainSettings &settings)
{
    m_settings = settings;
    setReplayGainInfo(m_info);
}

void ReplayGain::applyReplayGain(char *data, qint64 size)
{
    if(m_settings.mode() == ReplayGainSettings::DISABLED || m_scale == 1.0)
        return;
    size = size*8/m_bits;
    if(m_bits == 16)
    {
        for (qint64 i = 0; i < size; i++)
            ((short*)data)[i]*= m_scale;

    }
    else if(m_bits == 8)
    {
        for (qint64 i = 0; i < size; i++)
            ((char*)data)[i]*= m_scale;
    }
    else if(m_bits == 32)
    {
        for (qint64 i = 0; i < size; i++)
           ((qint32*)data)[i]*= m_scale;
    }
}

void ReplayGain::updateScale()
{
    double peak = 0.0;
    m_scale = 1.0;
    switch((int) m_settings.mode())
    {
    case ReplayGainSettings::TRACK:
        m_scale = pow(10.0, m_info[Qmmp::REPLAYGAIN_TRACK_GAIN]/20);
        peak = m_info[Qmmp::REPLAYGAIN_TRACK_PEAK];
        break;
    case ReplayGainSettings::ALBUM:
        m_scale = pow(10.0, m_info[Qmmp::REPLAYGAIN_ALBUM_GAIN]/20);
        peak = m_info[Qmmp::REPLAYGAIN_ALBUM_PEAK];
        break;
    case ReplayGainSettings::DISABLED:
        m_scale = 1.0;
        return;
    }
    if(m_scale == 1.0)
        m_scale = pow(10.0, m_settings.defaultGain()/20);
    m_scale *= pow(10.0, m_settings.preamp()/20);
    if(peak > 0.0 && m_settings.preventClipping())
        m_scale = m_scale*peak > 1.0 ? 1.0 / peak : m_scale;
    m_scale = qMin(m_scale, 5.6234); // +15 dB
    m_scale = qMax(m_scale, 0.1778);  // -15 dB
}
