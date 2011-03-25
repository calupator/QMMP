/***************************************************************************
 *   Copyright (C) 2010-2011 by Ilya Kotov                                 *
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

#ifndef EQSETTINGS_H
#define EQSETTINGS_H

/*! @brief The EqSettings class helps to work with equalizer settings.
 * @author Ilya Kotov <forkotov02@hotmail.ru>
 */
class EqSettings
{
public:
    /*!
     * Constructs an empty equalizer settings (0 dB, disabled)
     */
    EqSettings();
    /*!
     * Returns \b true if equalizer os enabled, otherwise returns \b false.
     */
    bool isEnabled() const;
    /*!
     * Returns gain of the channel number \b chan.
     */
    double gain(int chan) const;
    /*!
     * Returns equalizer preamp.
     */
    double preamp() const;
    /*!
     *  Enables equalizer if \p enabled is \b true or disables it if \p enabled is \b false.
     */
    void setEnabled(bool enabled = true);
    /*!
     * Sets gain of the equalizer channel.
     * @param chan Number of equalizer channel.
     * @param gain Channel gain (-20.0..20.0 dB)
     */
    void setGain(int chan, double gain);
    /*!
     * Sets equalizer preamp to \b preamp.
     */
    void setPreamp(double preamp);
    /*!
     * Assigns equalizer settings \b s to this settings.
     */
    void operator=(const EqSettings &s);
    /*!
     * Returns \b true if equalizer settins \b s is equal to this settings; otherwise returns false.
     */
    bool operator==(const EqSettings &s) const;
    /*!
     * Returns \b true if equalizer settins \b s is not equal to this settings; otherwise returns false.
     */
    bool operator!=(const EqSettings &s) const;

private:
    double m_gain[10];
    double m_preamp;
    bool m_is_enabled;
};

#endif // EQSETTINGS_H
