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

#ifndef kpgroupobject_h
#define kpgroupobject_h

class KPresenterDoc;

#include <kpobject.h>

/******************************************************************/
/* Class: KPGroupObject                                           */
/******************************************************************/

class KPGroupObject : public KPObject
{
public:
    KPGroupObject();
    KPGroupObject( const QPtrList<KPObject> &objs );
    virtual ~KPGroupObject() {}

    KPGroupObject &operator=( const KPGroupObject & );

    virtual void setSize( double _width, double _height );
    virtual void setOrig( KoPoint _point );
    virtual void setOrig( double _x, double _y );
    virtual void moveBy( KoPoint _point );
    virtual void moveBy( double _dx, double _dy );
    virtual void resizeBy( KoSize _size );
    virtual void resizeBy( double _dx, double _dy );

    virtual void rotate( float _angle );
    virtual void setShadowDistance( int _distance );
    virtual void setShadowDirection( ShadowDirection _direction );
    virtual void setShadowColor( const QColor &_color );
    virtual void setEffect( Effect _effect );
    virtual void setEffect2( Effect2 _effect2 );
    virtual void setPresNum( int _presNum );
    virtual void setDisappear( bool b );
    virtual void setDisappearNum( int num );
    virtual void setEffect3( Effect3 _effect3);
    virtual void setAppearTimer( int _appearTimer );
    virtual void setDisappearTimer( int _disappearTimer );
    virtual void setAppearSoundEffect( bool b );
    virtual void setDisappearSoundEffect( bool b );
    virtual void setAppearSoundEffectFileName( const QString &_a_fileName );
    virtual void setDisappearSoundEffectFileName( const QString &_d_fileName );

    virtual ObjType getType() const
    { return OT_GROUP; }
    virtual QString getTypeString() const
    { return i18n("Group"); }

    virtual QDomDocumentFragment save( QDomDocument& doc, int offset );
    virtual int load(const QDomElement &element, KPresenterDoc *doc);

    virtual void draw( QPainter *_painter, KoZoomHandler *_zoomhandler, bool drawSelection );

    void setUpdateObjects( bool b ) {
        updateObjs = b;
    }

    QPtrList<KPObject> getObjects() {
        return objects;
    }

    virtual void setOwnClipping( bool _ownClipping );
    virtual void setSubPresStep( int _subPresStep );
    virtual void doSpecificEffects( bool _specEffects, bool _onlyCurrStep = true );

protected:
    void updateSizes( double fx, double fy );
    void updateCoords( int dx, int dy );

    QPtrList<KPObject> objects;
    bool updateObjs;

};

#endif
