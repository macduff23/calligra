/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <math.h>

#include <qdom.h>

#include "vglobal.h"
#include "vsegment.h"
#include "vsegmentlist.h"

#include <kdebug.h>


// Calculate height of p above line AB.
static double
height(
	const KoPoint& a,
	const KoPoint& p,
	const KoPoint& b )
{
	// calculate determinant of AP and AB to obtain projection of vector AP to
	// the orthogonal vector of AB:
	const double det =
		p.x() * a.y() + b.x() * p.y() - p.x() * b.y() -
		a.x() * p.y() + a.x() * b.y() - b.x() * a.y();

	// calculate norm = length(AB):
	const double norm = sqrt(
		( b.x() - a.x() ) * ( b.x() - a.x() ) +
		( b.y() - a.y() ) * ( b.y() - a.y() ) );

	// if norm is very small, simply use distance AP:
	if( norm < VGlobal::verySmallNumber )
		return
			sqrt(
				( p.x() - a.x() ) * ( p.x() - a.x() ) +
				( p.y() - a.y() ) * ( p.y() - a.y() ) );

	// normalize:
	return QABS( det ) / norm;
}


VSegment::VSegment()
{
	m_prev = 0L;
	m_next = 0L;

	m_type = segment_begin;
	m_ctrlPointFixing = segment_none;
	m_smooth = false;
}

VSegment::VSegment( const VSegment& segment )
{
	// copying m_prev/m_next has some advantages ( see vsegment::length() ).
	// inserting a segment into a vsegmentlist overwrites these anyway:
	m_prev = segment.m_prev;
	m_next = segment.m_next;

	m_point[0] = segment.m_point[0];
	m_point[1] = segment.m_point[1];
	m_point[2] = segment.m_point[2];

	m_type = segment.m_type;
	m_ctrlPointFixing = segment.m_ctrlPointFixing;
	m_smooth = segment.m_smooth;
}

bool
VSegment::isFlat( double flatness ) const
{
	if(
		!m_prev ||
		m_type == segment_begin ||
		m_type == segment_line )
	{
		return true;
	}

	if( m_type == segment_curve )
		return
			height( m_prev->m_point[2], m_point[0], m_point[2] )
				< flatness &&
			height( m_prev->m_point[2], m_point[1], m_point[2] )
				< flatness;

	return false;
}

KoPoint
VSegment::point( double t ) const
{
	if( !m_prev || m_type == segment_begin )
		return KoPoint();

	// lines:
	if( m_type == segment_line )
	{
		return
			m_prev->m_point[2] +
			( m_point[2] - m_prev->m_point[2] ) * t;
	}

	// beziers:
	KoPoint q[4];
	q[0] = m_prev->m_point[2];
	q[1] = m_point[0];
	q[2] = m_point[1];
	q[3] = m_point[2];

	// de casteljau algorithm:
	for( uint j = 1; j <= 3; ++j )
	{
		for( uint i = 0; i <= 3 - j; ++i )
		{
			q[i] = ( 1.0 - t ) * q[i] + t * q[i+1];
		}
	}

	return q[0];
}

KoPoint
VSegment::derive( double t ) const
{
	if( !m_prev || m_type == segment_begin )
		return KoPoint();

	// lines:
	if( m_type == segment_line )
	{
		return
			( m_point[2] - m_prev->m_point[2] );
	}

	// beziers:
	KoPoint q[3];
	q[0] = m_point[0] - m_prev->m_point[2];
	q[1] = m_point[1] - m_point[0];
	q[2] = m_point[2] - m_point[1];

	// de casteljau algorithm:
	for( uint j = 1; j <= 2; ++j )
	{
		for( uint i = 0; i <= 2 - j; ++i )
		{
			q[i] = ( 1.0 - t ) * q[i] + t * q[i+1];
		}
	}

	return 3 * q[0];
}

KoPoint
VSegment::tangent( double t ) const
{
	KoPoint p( derive( t ) );

	const double norm =
		sqrt( p.x() * p.x() + p.y() * p.y() );

	return norm ? p * ( 1.0 / norm ) : KoPoint( 0.0, 0.0 );
}

void
VSegment::pointAndDerive( double t, KoPoint& p, KoPoint& der ) const
{
	if( !m_prev || m_type == segment_begin )
		return;

	// lines:
	if( m_type == segment_line )
	{
		der = m_point[2] - m_prev->m_point[2];
		p = m_prev->m_point[2] + der * t;

		return;
	}

	// beziers:
	KoPoint q[4];
	q[0] = m_prev->m_point[2];
	q[1] = m_point[0];
	q[2] = m_point[1];
	q[3] = m_point[2];

	// de casteljau algorithm:
	for( uint j = 1; j <= 3; ++j )
	{
		for( uint i = 0; i <= 3 - j; ++i )
		{
			q[i] = ( 1.0 - t ) * q[i] + t * q[i+1];
		}

		if( j == 2 )
			der = 3 * ( q[1] - q[0] );
	}

	p = q[0];

	return;
}

double
VSegment::length( double t ) const
{
	if( !m_prev )
		return 0.0;

	// length of a line:
	if( m_type == segment_line )
	{
		return
			sqrt(
				t * (
					( m_point[2].x() - m_prev->m_point[2].x() ) *
					( m_point[2].x() - m_prev->m_point[2].x() ) +
					( m_point[2].y() - m_prev->m_point[2].y() ) *
					( m_point[2].y() - m_prev->m_point[2].y() ) ) );
	}
	// length of a bezier:
	else if( m_type == segment_curve )
	{
		// This algortihm is based on an idea by Jens Gravesen <gravesen@mat.dth.dk>.
		// We calculate the chord length chord=|P0P3| and the length of the control point
		// polygon poly=|P0P1|+|P1P2|+|P2P3|. The approximation for the bezier length is
		// 0.5 * poly + 0.5 * chord. poly - chord is a measure for the error.
		// We subdivide each segment until the error is smaller than a given tolerance
		// and add up the subresults.


		// "copy segment" splitted a t into a vsegmentlist:
		VSegmentList list( 0L );
		list.moveTo( prev()->knot() );

		// most of the time we'll need the length of the whole segment:
		if( t == 1.0 )
			list.append( this->clone() );
		else
		{
			VSegment* copy = this->clone();
			list.append( copy->splitAt( t ) );
			delete copy;
		}


		double chord;
		double poly;

		double length = 0.0;

		while( list.current() )
		{
			chord =
				sqrt(
						( list.current()->m_point[2].x() -
							list.current()->m_prev->m_point[2].x() ) *
						( list.current()->m_point[2].x() -
							list.current()->m_prev->m_point[2].x() )
					+
						( list.current()->m_point[2].y() -
							list.current()->m_prev->m_point[2].y() ) *
						( list.current()->m_point[2].y() -
							list.current()->m_prev->m_point[2].y() ) );

			poly =
				sqrt(
						( list.current()->m_point[0].x() -
							list.current()->m_prev->m_point[2].x() ) *
						( list.current()->m_point[0].x() -
							list.current()->m_prev->m_point[2].x() )
					+
						( list.current()->m_point[0].y() -
							list.current()->m_prev->m_point[2].y() ) *
						( list.current()->m_point[0].y() -
							list.current()->m_prev->m_point[2].y() ) )
				+ sqrt(
						( list.current()->m_point[1].x() -
							list.current()->m_point[0].x() ) *
						( list.current()->m_point[1].x() -
							list.current()->m_point[0].x() )
					+
						( list.current()->m_point[1].y() -
							list.current()->m_point[0].y() ) *
						( list.current()->m_point[1].y() -
							list.current()->m_point[0].y() ) )
				+ sqrt(
						( list.current()->m_point[2].x() -
							list.current()->m_point[1].x() ) *
						( list.current()->m_point[2].x() -
							list.current()->m_point[1].x() )
					+
						( list.current()->m_point[2].y() -
							list.current()->m_point[1].y() ) *
						( list.current()->m_point[2].y() -
							list.current()->m_point[1].y() ) );

			if(
				poly &&
				( poly - chord ) / poly > VGlobal::lengthTolerance )
			{
				// split at midpoint:
				list.insert(
					list.current()->splitAt( 0.5 ) );
			}
			else
			{
				length += 0.5 * poly + 0.5 * chord;
				list.next();
			}
		}

		return length;
	}
	else
		return 0.0;
}

KoRect
VSegment::boundingBox() const
{
	// initialize with p3:
	KoRect rect( m_point[2], m_point[2] );

	if( m_prev )
	{
		if( m_prev->m_point[2].x() < rect.left() )
			rect.setLeft( m_prev->m_point[2].x() );
		if( m_prev->m_point[2].x() > rect.right() )
			rect.setRight( m_prev->m_point[2].x() );
		if( m_prev->m_point[2].y() < rect.top() )
			rect.setTop( m_prev->m_point[2].y() );
		if( m_prev->m_point[2].y() > rect.bottom() )
			rect.setBottom( m_prev->m_point[2].y() );
	}

	if( m_type == segment_curve )
	{
		if( m_point[0].x() < rect.left() )
			rect.setLeft( m_point[0].x() );
		if( m_point[0].x() > rect.right() )
			rect.setRight( m_point[0].x() );
		if( m_point[0].y() < rect.top() )
			rect.setTop( m_point[0].y() );
		if( m_point[0].y() > rect.bottom() )
			rect.setBottom( m_point[0].y() );

		if( m_point[1].x() < rect.left() )
			rect.setLeft( m_point[1].x() );
		if( m_point[1].x() > rect.right() )
			rect.setRight( m_point[1].x() );
		if( m_point[1].y() < rect.top() )
			rect.setTop( m_point[1].y() );
		if( m_point[1].y() > rect.bottom() )
			rect.setBottom( m_point[1].y() );
	}

	return rect;
}

VSegment*
VSegment::splitAt( double t )
{
	if( !m_prev || m_type == segment_begin )
		return 0L;

	VSegment* segment = new VSegment();

	// lines are easy: no need to change the current segment:
	if( m_type == segment_line )
	{
		segment->m_point[2] =
			m_prev->m_point[2] +
			( m_point[2] - m_prev->m_point[2] ) * t;

		segment->m_type = segment_line;
		return segment;
	}

	// these references make our life a bit easier:
	KoPoint& p0 = m_prev->m_point[2];
	KoPoint& p1 = m_point[0];
	KoPoint& p2 = m_point[1];
	KoPoint& p3 = m_point[2];

	// calculate the 2 new beziers:
	segment->m_point[0] = p0 + ( p1 - p0 ) * t;
	segment->m_point[1] = p1 + ( p2 - p1 ) * t;

	p2 = p2 + ( p3 - p2 ) * t;
	p1 = segment->m_point[1] + ( p2 - segment->m_point[1] ) * t;

	segment->m_point[1] =
		segment->m_point[0] + ( segment->m_point[1] - segment->m_point[0] ) * t;
	segment->m_point[2] =
		segment->m_point[1] + ( p1 - segment->m_point[1] ) * t;

	// set the new segment type:
	segment->m_type = segment_curve;

	return segment;
}

void
VSegment::convertToCurve( double t )
{
	if(
		!m_prev ||
		m_type == segment_begin )
	{
		return;
	}

	if( m_type == segment_line )
	{
		m_point[0] = point( t );
		m_point[1] = point( 1.0 - t );
	}

	m_type = segment_curve;
	m_ctrlPointFixing = segment_none;
}

bool
VSegment::linesIntersect(
	const KoPoint& a0,
	const KoPoint& a1,
	const KoPoint& b0,
	const KoPoint& b1 )
{
	const KoPoint delta_a = a1 - a0;
	const double det_a = a1.x() * a0.y() - a1.y() * a0.x();

	const double r_b0 = delta_a.y() * b0.x() - delta_a.x() * b0.y() + det_a;
	const double r_b1 = delta_a.y() * b1.x() - delta_a.x() * b1.y() + det_a;

	if( r_b0 != 0.0 && r_b1 != 0.0 && r_b0 * r_b1 > 0.0 )
		return false;

	const KoPoint delta_b = b1 - b0;
	const double det_b = b1.x() * b0.y() - b1.y() * b0.x();

	const double r_a0 = delta_b.y() * a0.x() - delta_b.x() * a0.y() + det_b;
	const double r_a1 = delta_b.y() * a1.x() - delta_b.x() * a1.y() + det_b;

	if( r_a0 != 0.0 && r_a1 != 0.0 && r_a0 * r_a1 > 0.0 )
		return false;

	return true;
}

void
VSegment::save( QDomElement& element ) const
{
	QDomElement me;
	element.appendChild( me );

	if( m_ctrlPointFixing )
		me.setAttribute( "ctrlPointFixing", m_ctrlPointFixing );

	if( m_type == segment_curve )
	{
		me = element.ownerDocument().createElement( "CURVE" );
		me.setAttribute( "x1", m_point[0].x() );
		me.setAttribute( "y1", m_point[0].y() );
		me.setAttribute( "x2", m_point[1].x() );
		me.setAttribute( "y2", m_point[1].y() );
		me.setAttribute( "x3", m_point[2].x() );
		me.setAttribute( "y3", m_point[2].y() );
	}
	else if( m_type == segment_line )
	{
		me = element.ownerDocument().createElement( "LINE" );
		me.setAttribute( "x", m_point[2].x() );
		me.setAttribute( "y", m_point[2].y() );
	}
	else if( m_type == segment_begin )
	{
		me = element.ownerDocument().createElement( "MOVE" );
		me.setAttribute( "x", m_point[2].x() );
		me.setAttribute( "y", m_point[2].y() );
	}
}

void
VSegment::load( const QDomElement& element )
{
	switch( element.attribute( "ctrlPointFixing", "0" ).toUShort() )
	{
		case 1:
			m_ctrlPointFixing = segment_first; break;
		case 2:
			m_ctrlPointFixing = segment_second; break;
		default:
			m_ctrlPointFixing = segment_none;
	}

	if( element.tagName() == "CURVE" )
	{
		m_type = segment_curve;
		m_point[0].setX( element.attribute( "x1" ).toDouble() );
		m_point[0].setY( element.attribute( "y1" ).toDouble() );
		m_point[1].setX( element.attribute( "x2" ).toDouble() );
		m_point[1].setY( element.attribute( "y2" ).toDouble() );
		m_point[2].setX( element.attribute( "x3" ).toDouble() );
		m_point[2].setY( element.attribute( "y3" ).toDouble() );
	}
	else if( element.tagName() == "LINE" )
	{
		m_type = segment_line;
		m_point[2].setX( element.attribute( "x" ).toDouble() );
		m_point[2].setY( element.attribute( "y" ).toDouble() );
	}
	else if( element.tagName() == "MOVE" )
	{
		m_type = segment_begin;
		m_point[2].setX( element.attribute( "x" ).toDouble() );
		m_point[2].setY( element.attribute( "y" ).toDouble() );
	}
}

VSegment*
VSegment::clone() const
{
	return new VSegment( *this );
}

