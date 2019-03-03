/***************************************************************************
 *   Copyright (C) 2008-2019 by Ilya Kotov                                 *
 *   forkotov02@ya.ru                                                      *
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
#ifndef GENERALFACTORY_H
#define GENERALFACTORY_H

#include "qmmpui_export.h"

class QObject;
class QDialog;
class QString;
class QWidget;
class Control;
class General;

/*! @brief Structure to store general plugin properies.
 */
struct GeneralProperties
{
    QString name;                   /*!< File dialog plugin full name */
    QString shortName;              /*!< File dialog short name for internal usage */
    bool hasAbout = false;          /*!< Should be \b true if plugin has about dialog, otherwise returns \b false */
    bool hasSettings = false;       /*!< Should be \b true if plugin has settings dialog, otherwise returns \b false */
    bool visibilityControl = false; /*!< Should be \b true if plugin can show/hide main window of the player,
                                     * otherwise returns \b false */
};
/*! @brief %General plugin interface.
 * @author Ilya Kotov <forkotov02@ya.ru>
 */
class GeneralFactory
{
public:
    /*!
     * Object destructor.
     */
    virtual ~GeneralFactory() {}
    /*!
     * Returns general plugin properties.
     */
    virtual GeneralProperties properties() const = 0;
    /*!
     * Creates object of the General class.
     */
    virtual QObject *create(QObject *parent) = 0;
    /*!
     * Creates configuration dialog.
     * @param parent Parent widget.
     * @return Configuration dialog pointer.
     */
    virtual QDialog *createConfigDialog(QWidget *parent) = 0;
    /*!
     * Shows about dialog.
     * @param parent Parent widget.
     */
    virtual void showAbout(QWidget *parent) = 0;
    /*!
     * Returns translation file path without locale code and extension
     */
    virtual QString translation() const = 0;
};

Q_DECLARE_INTERFACE(GeneralFactory, "GeneralFactory/1.0")
#endif
