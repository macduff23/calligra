/* This file is part of the KDE project
   Copyright (C) 2006 Rob Buis <buis@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOPATHGFXOBJ_H
#define KOPATHGFXOBJ_H

class KoSelection;

#include "KoShape.h"
#include <QPainterPath>

/**
 * Example/test flake object that draws a Shape based on a path.
 */
class KoPathShape : public KoShape
{
public:
    KoPathShape();

    virtual void paint( QPainter &painter, KoViewConverter &converter );
    virtual void paintDecorations(QPainter &painter, KoViewConverter &converter, bool selected);

    void moveTo( const QPointF &p );
    void lineTo( const QPointF &p );
    void curveTo( const QPointF &p1, const QPointF &p2, const QPointF &p );
    void close();

    bool hitTest( const QPointF &position ) const;
    virtual QRectF boundingRect() const;
    virtual const QPainterPath outline() const;

private:
    QPainterPath m_path;
};

#endif /* KOPATHGFXOBJ_H */

