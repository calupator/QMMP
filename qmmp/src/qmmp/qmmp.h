/***************************************************************************
 *   Copyright (C) 2008-2014 by Ilya Kotov                                 *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef QMMP_H
#define QMMP_H

#include <QString>

#define QMMP_VERSION_MAJOR 0
#define QMMP_VERSION_MINOR 9
#define QMMP_VERSION_PATCH 0
#define QMMP_VERSION_STABLE 0

#define QMMP_VERSION_INT (QMMP_VERSION_MAJOR<<16 | QMMP_VERSION_MINOR<<8 | QMMP_VERSION_PATCH)


/*! @brief The Qmmp class stores global settings and enums.
 * @author Ilya Kotov <forkotov02@hotmail.ru>
 */
class Qmmp
{
public:
    /*!
     * Playback state enum
     */
    enum State
    {
        Playing = 0, /*!< The player is playing source */
        Paused,      /*!< The player has currently paused its playback */
        Stopped,     /*!< The player is ready to play source */
        Buffering,   /*!< The Player is waiting for data to be able to start playing.   */
        NormalError, /*!< Input source is invalid or unsupported. Player should skip this file */
        FatalError   /*!< This means unrecorvable error die audio output problems. Player should abort playback. */
    };
    /*!
     * Metadata keys
     */
    enum MetaData
    {
        TITLE = 0, /*!< Title */
        ARTIST,    /*!< Artist  */
        ALBUMARTIST,/*!< Album artist  */
        ALBUM,     /*!< Album */
        COMMENT,   /*!< Comment */
        GENRE,     /*!< Genre */
        COMPOSER,  /*!< Composer */
        YEAR,      /*!< Year */
        TRACK,     /*!< Track number */
        DISCNUMBER,/*!< Disc number */
        URL        /*!< Stream url or local file path */
    };
    /*!
     * Keys of ReplayGain information
     */
    enum ReplayGainKey
    {
        REPLAYGAIN_TRACK_GAIN = 0, /*!< track gain */
        REPLAYGAIN_TRACK_PEAK,     /*!< track peak */
        REPLAYGAIN_ALBUM_GAIN,     /*!< album gain */
        REPLAYGAIN_ALBUM_PEAK      /*!< album peak */
    };
    /*!
     * Audio formats
     */
    enum AudioFormat
    {
        PCM_UNKNOWM = -1, /*!< Unknown format */
        PCM_S8 = 0, /*!< Signed 8 bit */
        PCM_S16LE,  /*!< Signed 16 bit Little Endian */
        PCM_S24LE,  /*!< Signed 24 bit Little Endian using low three bytes in 32-bit word */
        PCM_S32LE   /*!< Signed 32 bit Little Endian */
    };
    /*!
     * Returns the configuration file name, including the path.
     */
    static const QString configFile();
    /*!
     * Returns the configuration directory path.
     */
    static const QString configDir();
    /*!
     * Overrides default configuration directory path.
     */
    static void setConfigDir(const QString &path);
    /*!
     * Returns %Qmmp library version.
     */
    static const QString strVersion();
    /*!
     * Returns the location of the installed Qmmp plugins.
     */
    static const QString pluginsPath();
    /*!
     * Returns system language if uiLanguageID() is 'auto'. Otherwise returns uiLanguageID().
     */
    static QString systemLanguageID();
    /*!
     * Returns state of the user interface language option. Code "auto" means autodetection.
     */
    static QString uiLanguageID();
    /*!
     * Sets user interface language.
     * @param code Language code; code "auto" means autodetection.
     */
    static void setUiLanguageID(const QString &code);

private:
    static QString m_configDir;
    static QString m_langID;

};

#endif
