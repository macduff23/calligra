/* This file is part of the KDE project
   Copyright (C) 2000, 2001 Thomas Zander <zander@kde.org>

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

#ifndef KoBorder_h
#define KoBorder_h

#include <qcolor.h>
#include <qpen.h>
class QDomElement;
class KoZoomHandler;
class QPainter;

/**
 * This class represents a border - for anything, like paragraph, or frame
 */
class KoBorder : public Qt
{
public:
    enum BorderStyle {SOLID = 0, DASH = 1, DOT = 2, DASH_DOT = 3, DASH_DOT_DOT = 4, DOUBLE_LINE = 5};
    KoBorder();
    KoBorder( const QColor & c, BorderStyle s, double width );
    QColor color;
    void setPenWidth(double _w);
    void setStyle(BorderStyle _style);
    BorderStyle getStyle() const {return style;}
    double penWidth() const{ return ptPenWidth;}
    double width() const { return ptWidth; }

    bool operator==( const KoBorder _brd ) const;
    bool operator!=( const KoBorder _brd ) const;

    // Load from XML
    static KoBorder loadBorder( const QDomElement & elem );
    // Save to XML
    void save( QDomElement & elem ) const;

    // String to style enum, and vice versa, for UI.
    static BorderStyle getStyle( const QString &style );
    static QString getStyle( const BorderStyle &style );

    // Zoom the border width. If ptWidth is 0, minborder is returned.
    static int zoomWidthX( double ptWidth, KoZoomHandler * zoomHandler, int minborder ); // For left/right borders
    static int zoomWidthY( double ptWidth, KoZoomHandler * zoomHandler, int minborder ); // For top/bottom borders

    // Get a ready-to-use QPen for this border.
    static QPen borderPen( const KoBorder & brd, int width, QColor defaultColor );

    // The do-it-all method :)
    // Draws in @p painter the 4 borders on the _outside_ of @p rect.
    // If a border is of size 0, minborder will be applied (no border if 0, defaultPen otherwise)
    // defaultColor is the color to use for QColor() - either defaultTextColor() or defaultBgColor().
    static void drawBorders( QPainter& painter, KoZoomHandler * zoomHandler, QRect rect,
                             KoBorder left, KoBorder right, KoBorder top, KoBorder bottom,
                             int minborder, QPen defaultPen );
private:
    double ptWidth;
    double ptPenWidth;
    BorderStyle style;
};

#endif
