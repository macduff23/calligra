/* This file is part of the Calligra project, made within the KDE community.

   Copyright 2012 Friedrich W. H. Kossebau <kossebau@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "formatdocument.h"


Structure
FormatDocument::structure( const QString& name ) const
{
    Structure result;

    foreach( const Structure& structure, mStructures )
    {
        if( structure.name() == name )
        {
            result = structure;
            break;
        }
    }

    return result;
}

int
FormatDocument::sizeOfType(const QString& typeName) const
{
    int result = -1;

    const QString* actualTypeName = &typeName;

    QHash<QString,QString>::ConstIterator it = mTypeDefByName.find(typeName);
    if( it != mTypeDefByName.constEnd() )
        actualTypeName = &(it.value());

    foreach( const IncludedType& includedType, mIncludedTypes )
    {
        if( includedType.name() == *actualTypeName )
        {
            result = includedType.size();
            break;
        }
    }

    if( result == -1 )
    {
        foreach( const Structure& structure, mStructures )
        {
            if( structure.name() == *actualTypeName )
            {
                result = structure.size();
                break;
            }
        }
    }

    if( result == -1 )
    {
        struct BuiltInTypeDatum { char const* name; int size; };
        static const BuiltInTypeDatum builtInTypeData[] =
        {
            {"char", 1},
            {"unsigned char", 1},
            {"float", 4}
        };
        static const int builtInTypeDataSize = sizeof( builtInTypeData ) / sizeof( builtInTypeData[0] );
        for( int i = 0; i < builtInTypeDataSize; ++i)
        {
            const BuiltInTypeDatum& builtInTypeDatum = builtInTypeData[i];
            if( builtInTypeDatum.name == *actualTypeName )
            {
                result = builtInTypeDatum.size;
                break;
            }
        }
    }

    return result;
}
