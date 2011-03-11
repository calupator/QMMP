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
#ifndef LADSPASLIDER_H
#define LADSPASLIDER_H

#include <QWidget>
#include "ladspa.h"

class QDoubleSpinBox;
class QSlider;

/**
    @author Ilya Kotov <forkotov02@hotmail.ru>
*/
class LADSPASlider : public QWidget
{
Q_OBJECT
public:
    LADSPASlider(double min,
                 double max,
                 double step,
                 LADSPA_Data *value,
                 QWidget *parent = 0);

private slots:
    void setValue(double);
    void setValue(int);

private:
   QDoubleSpinBox *m_spinBox;
   QSlider *m_slider;
   LADSPA_Data *m_value;
   double m_min;
   double m_max;
   double m_step;
};

#endif // LADSPASLIDER_H
