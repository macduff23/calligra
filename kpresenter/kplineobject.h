/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef kplineobject_h
#define kplineobject_h

#include <kpobject.h>

#define RAD_FACTOR 180.0 / M_PI

class QPainter;
class DCOPObject;

/******************************************************************/
/* Class: KPLineObject                                            */
/******************************************************************/

class KPLineObject : public KPShadowObject
{
public:
    KPLineObject();
    KPLineObject( const QPen &_pen, LineEnd _lineBegin, LineEnd _lineEnd, LineType _lineType );
    virtual ~KPLineObject()
    {}

    virtual DCOPObject* dcopObject();

    KPLineObject &operator=( const KPLineObject & );

    virtual void setLineBegin( LineEnd _lineBegin )
    { lineBegin = _lineBegin; }
    virtual void setLineEnd( LineEnd _lineEnd )
    { lineEnd = _lineEnd; }
    virtual void setLineType( LineType _lineType )
    { lineType = _lineType; }

    virtual ObjType getType() const
    { return OT_LINE; }
    virtual QString getTypeString() const
    { return i18n("Line"); }
    virtual LineEnd getLineBegin() const
    { return lineBegin; }
    virtual LineEnd getLineEnd() const
    { return lineEnd; }
    virtual LineType getLineType() const
    { return lineType; }

    virtual QDomDocumentFragment save( QDomDocument& doc, double offset );
    virtual double load(const QDomElement &element);

protected:
    float getAngle( const KoPoint &p1, const KoPoint &p2 );
    virtual void paint( QPainter *_painter, KoZoomHandler*_zoomHandler,
			bool drawingShadow, bool drawContour = FALSE );

    LineEnd lineBegin, lineEnd;
    LineType lineType;

};

#endif
