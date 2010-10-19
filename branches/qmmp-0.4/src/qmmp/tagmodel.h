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

#ifndef TAGMODEL_H
#define TAGMODEL_H

#include <QString>
#include <QList>
#include "qmmp.h"

/*! @brief The StateHandler class provides is the base interface class of tag editor.
 * @author Ilya Kotov <forkotov02@hotmail.ru>
 */
class TagModel
{
public:
    /*!
     * This enum describes tag editor capabilities
     */
    enum Caps
    {
        NoOptions = 0x0,    /*!< No capabilities */
        CreateRemove = 0x1, /*!< Can create/remove tag */
        Save = 0x2,         /*!< Can save changes */
    };
    /*!
     * Constructor.
     * @param f Capabilities.
     */
    TagModel(int f = TagModel::CreateRemove | TagModel::Save);
    /*!
     * Destructor.
     */
    virtual ~TagModel();
    /*!
     * Returns tag name.
     * Subclass should reimplement this fucntion.
     */
    virtual const QString name() = 0;
    /*!
     * Returns available keys. Default implementations returns all possible keys.
     */
    virtual QList<Qmmp::MetaData> keys();
    /*!
     * Returns the metdata string associated with the given \b key.
     * Subclass should reimplement this fucntion.
     */
    virtual const QString value(Qmmp::MetaData key) = 0;
    /*!
     * Changes metadata string associated with the given \b key to \b value.
     * Subclass should reimplement this fucntion.
     */
    virtual void setValue(Qmmp::MetaData key, const QString &value) = 0;
    /*!
     * Changes metadata string associated with the given \b key to \b value.
     * Subclass should reimplement this fucntion.
     */
    virtual void setValue(Qmmp::MetaData key, int value);
    /*!
     * Returns \b true if this tag exists; otherwise returns \b false.
     */
    virtual bool exists();
    /*!
     * Creates tag.
     */
    virtual void create();
    /*!
     * Removes tag.
     */
    virtual void remove();
    /*!
     * Saves tag.
     */
    virtual void save();
    /*!
     * Returns capability flags.
     */
    int caps();

private:
    int m_f;
};

#endif // TAGMODEL_H
