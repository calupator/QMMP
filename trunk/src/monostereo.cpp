/***************************************************************************
 *   Copyright (C) 2006 by Ilya Kotov                                      *
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
#include <QPainter>

#include "skin.h"
#include "monostereo.h"

MonoStereo::MonoStereo ( QWidget *parent )
      : PixmapWidget ( parent )
{
   m_skin = Skin::getPointer();
   m_pixmap = QPixmap ( 54,12 );
   setChannels ( 0 );
   connect(m_skin, SIGNAL(skinChanged()), this, SLOT(updateSkin()));
}


MonoStereo::~MonoStereo()
{}

void MonoStereo::setChannels ( int c )
{
   m_channels  = c;
   QPainter paint ( &m_pixmap );
   switch ( ( int ) c )
   {
      case 0:
      {
         paint.drawPixmap ( 0,0,m_skin->getMSPart ( Skin::MONO_I ) );
         paint.drawPixmap ( 27,0,m_skin->getMSPart ( Skin::STEREO_I ) );
         break;
      }
      case 1:
      {
         paint.drawPixmap ( 0,0,m_skin->getMSPart ( Skin::MONO_A ) );
         paint.drawPixmap ( 27,0,m_skin->getMSPart ( Skin::STEREO_I ) );
         break;
      }
   }
   if ( c > 1 )
   {
      paint.drawPixmap ( 0,0,m_skin->getMSPart ( Skin::MONO_I ) );
      paint.drawPixmap ( 27,0,m_skin->getMSPart ( Skin::STEREO_A ) );
   }
   setPixmap ( m_pixmap );
}

void MonoStereo::updateSkin()
{
   setChannels ( m_channels );
}
