/***************************************************************************
 *   Copyright (C) 2009-2010 by Ilya Kotov                                 *
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

#ifndef METADATAMANAGER_H
#define METADATAMANAGER_H

#include <QList>
#include <QStringList>
#include <QPixmap>
#include <QDir>
#include "fileinfo.h"
#include "metadatamodel.h"

class DecoderFactory;
class EngineFactory;
class InputSourceFactory;
class QmmpSettings;

/*! @brief The MetaDataManager class is the base class for metadata access.
 * @author Ilya Kotov <forkotov02@hotmail.ru>
 */
class MetaDataManager
{
public:
    /*!
     * Constructor. Use MetaDataManager::instance() instead.
     */
    MetaDataManager();
    /*!
     * Destructor. Use MetaDataManager::destroy() instead.
     */
    ~MetaDataManager();
    /*!
     * Extracts metadata and audio information from file \b path and returns a list of FileInfo items.
     * One file may contain several playlist items (for example: cda disk or flac with embedded cue)
     * @param path Source file path.
     * @param useMetaData Metadata usage (\b true - use, \b - do not use)
     */
    QList <FileInfo *> createPlayList(const QString &path, bool useMetaData = true) const;
    /*!
     * Creats metadata object, which provides full access to file tags.
     * @param url File path or URL.
     * @param parent Parent object.
     * @return MetaDataModel pointer or null pointer.
     */
    MetaDataModel* createMetaDataModel(const QString &url, QObject *parent = 0) const;
    /*!
     * Returns a list of file name filters with description, i.e. "MPEG Files (*.mp3 *.mpg)"
     */
    QStringList filters() const;
    /*!
     * Returns a list of file name filters, i.e. "*.mp3 *.mpg"
     */
    QStringList nameFilters() const;
    /*!
     * Returns a list of the suported protocols
     */
    QStringList protocols() const;
    /*!
     * Returns \b true if \b file is supported and exists, otherwise returns \b false
     */
    bool supports(const QString &file) const;
    /*!
     * Returns cover pixmap for the given file \b fileName,
     * or returns an empty pixmap if cover is not available.
     */
    QPixmap getCover(const QString &fileName);
    /*!
     * Returns cover file path for the given file \b fileName, or returns
     * an empty string if cover file is not available. This function does not work
     * with embedded covers.
     */
    QString getCoverPath(const QString &fileName);
    /*!
     * Clears cover path cache.
     */
    void clearCoverChache();
    /*!
     * Returns a pointer to the MetaDataManager instance.
     */
    static MetaDataManager* instance();
    /*!
     * Destroys MetaDataManager object.
     */
    static void destroy();

private:
    QFileInfoList findCoverFiles(QDir dir, int depth) const;
    QList <DecoderFactory *> *m_decoderFactories;
    QList <EngineFactory *> *m_engineFactories;
    QList <InputSourceFactory *> *m_inputSourceFactories;
    QMap <QString, QString> m_cover_cache;
    QmmpSettings *m_settings;
    static MetaDataManager* m_instance;
};

#endif // METADATAMANAGER_H
