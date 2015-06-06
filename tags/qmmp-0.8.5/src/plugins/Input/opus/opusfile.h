/***************************************************************************
    copyright            : (C) 2012 by Lukáš Lalinský
    email                : lalinsky@gmail.com

    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
                           (original Vorbis implementation)
***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

/*
  Changes against original code:
  - removed constructor File(IOStream *stream, bool readProperties, Properties::ReadStyle propertiesStyle);
  - removed function PropertyMap File::properties() const;
  - removed function PropertyMap File::setProperties(const PropertyMap &properties).
*/

#ifndef TAGLIB_OPUSFILE_H
#define TAGLIB_OPUSFILE_H

#include <taglib/oggfile.h>
#include <taglib/xiphcomment.h>

#include "opusproperties.h"

namespace TagLib {

  namespace Ogg {

    //! A namespace containing classes for Opus metadata

    namespace Opus {

      //! An implementation of Ogg::File with Opus specific methods

      /*!
       * This is the central class in the Ogg Opus metadata processing collection
       * of classes.  It's built upon Ogg::File which handles processing of the Ogg
       * logical bitstream and breaking it down into pages which are handled by
       * the codec implementations, in this case Opus specifically.
       */

      class File : public Ogg::File
      {
      public:
        /*!
         * Contructs a Opus file from \a file.  If \a readProperties is true the
         * file's audio properties will also be read using \a propertiesStyle.  If
         * false, \a propertiesStyle is ignored.
         */
        File(FileName file, bool readProperties = true,
             Properties::ReadStyle propertiesStyle = Properties::Average);

        /*!
         * Destroys this instance of the File.
         */
        virtual ~File();

        /*!
         * Returns the XiphComment for this file.  XiphComment implements the tag
         * interface, so this serves as the reimplementation of
         * TagLib::File::tag().
         */
        virtual Ogg::XiphComment *tag() const;

        /*!
         * Returns the Opus::Properties for this file.  If no audio properties
         * were read then this will return a null pointer.
         */
        virtual Properties *audioProperties() const;

        virtual bool save();

      private:
        File(const File &);
        File &operator=(const File &);

        void read(bool readProperties, Properties::ReadStyle propertiesStyle);

        class FilePrivate;
        FilePrivate *d;
      };
    }
  }
}

#endif
